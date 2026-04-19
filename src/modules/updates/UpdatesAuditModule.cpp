#include "modules/updates/UpdatesAuditModule.h"

#include "core/StringUtils.h"
#include "platform/windows/Encoding.h"
#include "platform/windows/ProcessRunner.h"
#include "platform/windows/RegistryUtils.h"

#include <windows.h>

#include <string>
#include <vector>

namespace {

std::string mapAuOptions(const DWORD value) {
    switch (value) {
        case 2:
            return "Уведомлять перед загрузкой";
        case 3:
            return "Автозагрузка и уведомление перед установкой";
        case 4:
            return "Автозагрузка и установка по расписанию";
        case 5:
            return "Выбор оставлен локальному администратору";
        case 7:
            return "Уведомлять об установке и перезапуске";
        default:
            return "Неизвестно";
    }
}

}  // namespace

namespace sec {

std::vector<AuditItem> UpdatesAuditModule::collect() const {
    std::vector<AuditItem> items;

    AuditItem updateConfig;
    updateConfig.category = "Обновления";
    updateConfig.title = "Конфигурация Windows Update";

    const auto noAutoUpdate = readRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU", L"NoAutoUpdate");
    const auto policyAuOptions = readRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU", L"AUOptions");
    const auto localAuOptions = readRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update", L"AUOptions");
    const auto lastSuccess = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\Results\\Install", L"LastSuccessTime");
    const auto rebootRequired = registryKeyExists(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired");

    updateConfig.details.push_back("Автоматические обновления отключены политикой: " + formatBool(noAutoUpdate.value_or(0) != 0, "да", "нет"));
    if (policyAuOptions.has_value()) {
        updateConfig.details.push_back("AUOptions по политике: " + mapAuOptions(policyAuOptions.value()));
    } else if (localAuOptions.has_value()) {
        updateConfig.details.push_back("Локальный AUOptions: " + mapAuOptions(localAuOptions.value()));
    } else {
        updateConfig.details.push_back("AUOptions: недоступно");
    }
    updateConfig.details.push_back("Требуется перезагрузка после обновлений: " + formatBool(rebootRequired, "да", "нет"));
    updateConfig.details.push_back("Последняя успешная установка: " + (lastSuccess.has_value() ? toUtf8(lastSuccess.value()) : std::string("неизвестно")));

    const auto weakConfig = noAutoUpdate.value_or(0) != 0 || !lastSuccess.has_value();
    updateConfig.status = weakConfig ? AuditStatus::warning : AuditStatus::ok;
    updateConfig.recommendation = weakConfig
        ? "Держи автоматические обновления включенными и контролируй регулярную успешную установку патчей."
        : "Конфигурация Windows Update выглядит нормально.";
    items.push_back(std::move(updateConfig));

    AuditItem hotfixes;
    hotfixes.category = "Обновления";
    hotfixes.title = "Последние установленные hotfix";

    const ProcessRunner runner;
    const auto result = runner.runPowerShell(
        "Get-HotFix | Sort-Object InstalledOn -Descending | Select-Object -First 5 | ForEach-Object { "
        "\"{0}`t{1:yyyy-MM-dd}`t{2}\" -f $_.HotFixID,$_.InstalledOn,$_.Description }");

    if (!result.success) {
        hotfixes.status = AuditStatus::error;
        hotfixes.details = {"Не удалось получить историю hotfix: " + trim(result.errorMessage)};
        hotfixes.recommendation = "Проверь PowerShell и WMI-доступность для инвентаризации обновлений.";
    } else {
        for (const auto& line : splitLines(result.output)) {
            if (trim(line).empty()) {
                continue;
            }
            const auto fields = split(line, '\t');
            if (fields.size() < 3) {
                continue;
            }
            hotfixes.details.push_back(trim(fields[0]) + " :: " + trim(fields[1]) + " :: " + trim(fields[2]));
        }
        hotfixes.status = hotfixes.details.empty() ? AuditStatus::warning : AuditStatus::ok;
        hotfixes.recommendation = hotfixes.details.empty()
            ? "Свежие записи hotfix не получены. Проверь историю обновлений."
            : "Сверь частоту установки обновлений с вашей patch-политикой.";
    }
    items.push_back(std::move(hotfixes));

    return items;
}

}  // namespace sec
