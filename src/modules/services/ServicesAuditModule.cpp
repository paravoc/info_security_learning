#include "modules/services/ServicesAuditModule.h"

#include "platform/windows/WinError.h"

#include <Windows.h>

#include <string>
#include <vector>

namespace {

struct ServiceInfo {
    bool exists {false};
    bool running {false};
    DWORD startType {0};
    std::string stateText;
    std::string startTypeText;
};

std::string mapStartType(const DWORD startType) {
    switch (startType) {
        case SERVICE_AUTO_START:
            return "Автоматически";
        case SERVICE_BOOT_START:
            return "Boot";
        case SERVICE_DEMAND_START:
            return "Вручную";
        case SERVICE_DISABLED:
            return "Отключена";
        case SERVICE_SYSTEM_START:
            return "System";
        default:
            return "Неизвестно";
    }
}

ServiceInfo queryService(const std::wstring& serviceName) {
    ServiceInfo info;

    const auto scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm == nullptr) {
        info.stateText = "SCM недоступен: " + sec::formatWindowsErrorMessage(GetLastError());
        return info;
    }

    const auto service = OpenServiceW(scm, serviceName.c_str(), SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (service == nullptr) {
        CloseServiceHandle(scm);
        info.stateText = "Служба отсутствует";
        return info;
    }

    info.exists = true;

    DWORD bytesNeeded = 0;
    SERVICE_STATUS_PROCESS status {};
    if (QueryServiceStatusEx(service,
                             SC_STATUS_PROCESS_INFO,
                             reinterpret_cast<LPBYTE>(&status),
                             sizeof(status),
                             &bytesNeeded)) {
        info.running = status.dwCurrentState == SERVICE_RUNNING;
        info.stateText = info.running ? "Запущена" : "Не запущена";
    } else {
        info.stateText = "Состояние недоступно: " + sec::formatWindowsErrorMessage(GetLastError());
    }

    QueryServiceConfigW(service, nullptr, 0, &bytesNeeded);
    std::vector<BYTE> buffer(bytesNeeded);
    auto* config = reinterpret_cast<QUERY_SERVICE_CONFIGW*>(buffer.data());
    if (QueryServiceConfigW(service, config, bytesNeeded, &bytesNeeded)) {
        info.startType = config->dwStartType;
        info.startTypeText = mapStartType(config->dwStartType);
    } else {
        info.startTypeText = "Неизвестно";
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return info;
}

}  // namespace

namespace sec {

std::vector<AuditItem> ServicesAuditModule::collect() const {
    std::vector<AuditItem> items;

    AuditItem core;
    core.category = "Службы";
    core.title = "Базовые защитные службы";

    const struct {
        std::wstring name;
        std::string label;
        bool shouldRun;
        bool shouldNotBeDisabled;
    } coreServices[] {
        {L"BFE", "Base Filtering Engine", true, true},
        {L"MpsSvc", "Windows Firewall", true, true},
        {L"WinDefend", "Microsoft Defender", true, true},
        {L"wuauserv", "Windows Update", false, true},
    };

    bool weakCore = false;
    for (const auto& service : coreServices) {
        const auto info = queryService(service.name);
        core.details.push_back(service.label + " :: " + info.stateText + ", start=" + info.startTypeText);

        if (!info.exists) {
            weakCore = true;
            continue;
        }

        if (service.shouldRun && !info.running) {
            weakCore = true;
        }
        if (service.shouldNotBeDisabled && info.startType == SERVICE_DISABLED) {
            weakCore = true;
        }
    }

    core.status = weakCore ? AuditStatus::warning : AuditStatus::ok;
    core.recommendation = weakCore
        ? "Убедись, что firewall, filtering, Defender и службы обновлений не отключены и что базовая защита активна."
        : "Ключевые защитные службы Windows выглядят нормально.";
    items.push_back(std::move(core));

    AuditItem remote;
    remote.category = "Службы";
    remote.title = "Удаленное администрирование и шаринг";

    const struct {
        std::wstring name;
        std::string label;
    } remoteServices[] {
        {L"RemoteRegistry", "Remote Registry"},
        {L"WinRM", "Windows Remote Management"},
        {L"sshd", "OpenSSH Server"},
        {L"TermService", "Remote Desktop Services"},
        {L"LanmanServer", "File and Printer Sharing"},
    };

    bool remoteEnabled = false;
    for (const auto& service : remoteServices) {
        const auto info = queryService(service.name);
        remote.details.push_back(service.label + " :: " + info.stateText + ", start=" + info.startTypeText);
        if (info.exists && info.running) {
            remoteEnabled = true;
        }
    }

    remote.status = remoteEnabled ? AuditStatus::warning : AuditStatus::ok;
    remote.recommendation = remoteEnabled
        ? "Проверь, какие службы удаленного управления и шаринга реально нужны на этом хосте."
        : "В выбранном списке не найдено активных служб удаленного управления.";
    items.push_back(std::move(remote));

    return items;
}

}  // namespace sec
