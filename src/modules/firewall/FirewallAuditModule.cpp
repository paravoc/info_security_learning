#include "modules/firewall/FirewallAuditModule.h"

#include "core/StringUtils.h"
#include "platform/windows/ProcessRunner.h"

#include <string>
#include <vector>

namespace sec {

std::vector<AuditItem> FirewallAuditModule::collect() const {
    const ProcessRunner runner;
    const auto result = runner.runPowerShell(
        "Get-NetFirewallProfile | Sort-Object Name | ForEach-Object { "
        "\"{0}`t{1}`t{2}`t{3}`t{4}`t{5}\" -f $_.Name,$_.Enabled,$_.DefaultInboundAction,$_.DefaultOutboundAction,$_.AllowInboundRules,$_.AllowLocalFirewallRules }");

    AuditItem profiles;
    profiles.category = "Firewall";
    profiles.title = "Профили межсетевого экрана";

    if (!result.success) {
        profiles.status = AuditStatus::error;
        profiles.details = {"Не удалось получить профили firewall: " + trim(result.errorMessage)};
        profiles.recommendation = "Проверь PowerShell-модули сети и локальные ограничения политик.";
        return {profiles};
    }

    bool weakProfile = false;
    for (const auto& line : splitLines(result.output)) {
        if (trim(line).empty()) {
            continue;
        }

        const auto fields = split(line, '\t');
        if (fields.size() < 6) {
            continue;
        }

        const auto enabled = trim(fields[1]) == "True";
        const auto inboundAction = trim(fields[2]);
        const auto outboundAction = trim(fields[3]);
        profiles.details.push_back(
            trim(fields[0]) + " :: enabled=" + trim(fields[1]) +
            ", inbound=" + inboundAction +
            ", outbound=" + outboundAction +
            ", allowInboundRules=" + trim(fields[4]) +
            ", allowLocalRules=" + trim(fields[5]));

        if (!enabled || inboundAction == "Allow") {
            weakProfile = true;
        }
    }

    profiles.status = weakProfile ? AuditStatus::warning : AuditStatus::ok;
    profiles.recommendation = weakProfile
        ? "Включи все релевантные профили firewall и по возможности держи входящее действие в режиме Block."
        : "Профили firewall выглядят достаточно строгими.";
    return {profiles};
}

}  // namespace sec
