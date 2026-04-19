#include "modules/system/SystemAuditModule.h"

#include "core/StringUtils.h"
#include "platform/windows/Encoding.h"
#include "platform/windows/WinError.h"

#include <windows.h>
#include <winternl.h>

#include <string>
#include <vector>

namespace {

std::wstring queryComputerName() {
    DWORD size = 0;
    GetComputerNameExW(ComputerNameDnsHostname, nullptr, &size);
    std::wstring name(size, L'\0');
    if (!GetComputerNameExW(ComputerNameDnsHostname, name.data(), &size)) {
        return L"Неизвестно";
    }
    name.resize(size);
    return name;
}

std::wstring queryUserName() {
    DWORD size = 0;
    GetUserNameW(nullptr, &size);
    std::wstring name(size, L'\0');
    if (!GetUserNameW(name.data(), &size)) {
        return L"Неизвестно";
    }
    if (!name.empty() && name.back() == L'\0') {
        name.pop_back();
    }
    return name;
}

std::string queryWindowsVersion() {
    using RtlGetVersionPtr = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

    const auto ntdll = GetModuleHandleW(L"ntdll.dll");
    const auto rtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(ntdll, "RtlGetVersion"));

    RTL_OSVERSIONINFOW info {};
    info.dwOSVersionInfoSize = sizeof(info);

    if (rtlGetVersion == nullptr || rtlGetVersion(&info) != 0) {
        return "Версия Windows недоступна";
    }

    return "Windows " + std::to_string(info.dwMajorVersion) + "." + std::to_string(info.dwMinorVersion) +
           " (build " + std::to_string(info.dwBuildNumber) + ")";
}

std::string queryMemoryLine() {
    MEMORYSTATUSEX memory {};
    memory.dwLength = sizeof(memory);
    if (!GlobalMemoryStatusEx(&memory)) {
        return "Информация о памяти недоступна";
    }

    return "ОЗУ всего: " + sec::humanReadableBytes(memory.ullTotalPhys) +
           ", доступно: " + sec::humanReadableBytes(memory.ullAvailPhys);
}

DWORD queryRegistryDword(const std::wstring& subKey, const std::wstring& valueName, DWORD fallbackValue) {
    DWORD value = fallbackValue;
    DWORD size = sizeof(value);
    const auto status = RegGetValueW(HKEY_LOCAL_MACHINE,
                                     subKey.c_str(),
                                     valueName.c_str(),
                                     RRF_RT_REG_DWORD,
                                     nullptr,
                                     &value,
                                     &size);
    return status == ERROR_SUCCESS ? value : fallbackValue;
}

std::string queryServiceState(const std::wstring& serviceName, bool& isRunning) {
    isRunning = false;

    const auto scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm == nullptr) {
        return "Не удалось открыть SCM: " + sec::formatWindowsErrorMessage(GetLastError());
    }

    const auto service = OpenServiceW(scm, serviceName.c_str(), SERVICE_QUERY_STATUS);
    if (service == nullptr) {
        const auto errorText = sec::formatWindowsErrorMessage(GetLastError());
        CloseServiceHandle(scm);
        return "Не удалось открыть службу: " + errorText;
    }

    SERVICE_STATUS_PROCESS status {};
    DWORD bytesNeeded = 0;
    const auto ok = QueryServiceStatusEx(service,
                                         SC_STATUS_PROCESS_INFO,
                                         reinterpret_cast<LPBYTE>(&status),
                                         sizeof(status),
                                         &bytesNeeded);

    std::string result;
    if (!ok) {
        result = "Не удалось получить состояние службы: " + sec::formatWindowsErrorMessage(GetLastError());
    } else {
        isRunning = status.dwCurrentState == SERVICE_RUNNING;
        result = isRunning ? "Запущена" : "Не запущена";
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return result;
}

}  // namespace

namespace sec {

std::vector<AuditItem> SystemAuditModule::collect() const {
    std::vector<AuditItem> items;

    AuditItem profile;
    profile.category = "Система";
    profile.title = "Профиль хоста";
    profile.status = AuditStatus::info;
    profile.details = {
        "Имя компьютера: " + toUtf8(queryComputerName()),
        "Текущий пользователь: " + toUtf8(queryUserName()),
        "Версия: " + queryWindowsVersion(),
        "Аптайм: " + humanReadableDurationSeconds(GetTickCount64() / 1000),
        queryMemoryLine(),
    };
    items.push_back(std::move(profile));

    const auto enableLua = queryRegistryDword(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"EnableLUA", 1);
    AuditItem uac;
    uac.category = "Система";
    uac.title = "Состояние UAC";
    uac.status = enableLua == 0 ? AuditStatus::warning : AuditStatus::ok;
    uac.details = {"Контроль учетных записей (UAC): " + formatBool(enableLua != 0, "включен", "выключен")};
    uac.recommendation = enableLua == 0 ? "Включи UAC для более безопасной административной работы." : "Держи UAC включенным для контролируемого повышения прав.";
    items.push_back(std::move(uac));

    const auto denyRdp = queryRegistryDword(L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server", L"fDenyTSConnections", 1);
    AuditItem rdp;
    rdp.category = "Удаленный доступ";
    rdp.title = "Экспозиция Remote Desktop";
    rdp.status = denyRdp == 0 ? AuditStatus::warning : AuditStatus::ok;
    rdp.details = {"Remote Desktop: " + formatBool(denyRdp == 0, "включен", "выключен")};
    rdp.recommendation = denyRdp == 0 ? "Если RDP не нужен, отключи его или ограничь firewall-правилами и MFA." : "Прямой экспозиции RDP не обнаружено.";
    items.push_back(std::move(rdp));

    bool defenderRunning = false;
    const auto defenderState = queryServiceState(L"WinDefend", defenderRunning);
    AuditItem defender;
    defender.category = "Защита";
    defender.title = "Служба Microsoft Defender";
    defender.status = defenderRunning ? AuditStatus::ok : AuditStatus::warning;
    defender.details = {"Состояние службы: " + defenderState};
    defender.recommendation = defenderRunning ? "Продолжай контролировать сигнатуры и статус обновлений Defender." : "Проверь, не установлен ли другой защитный продукт и активна ли его защита.";
    items.push_back(std::move(defender));

    return items;
}

}  // namespace sec
