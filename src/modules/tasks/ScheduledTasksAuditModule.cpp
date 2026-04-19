#include "modules/tasks/ScheduledTasksAuditModule.h"

#include "core/StringUtils.h"
#include "platform/windows/ProcessRunner.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace {

bool containsAny(const std::string& value, const std::vector<std::string>& markers) {
    return std::any_of(markers.begin(), markers.end(), [&](const std::string& marker) {
        return value.find(marker) != std::string::npos;
    });
}

}  // namespace

namespace sec {

std::vector<AuditItem> ScheduledTasksAuditModule::collect() const {
    std::vector<AuditItem> items;

    const ProcessRunner runner;
    const auto result = runner.runPowerShell(
        "Get-ScheduledTask | ForEach-Object { "
        "$actions = ($_.Actions | ForEach-Object { $_.Execute }) -join ';'; "
        "$level = if ($_.Principal -and $_.Principal.RunLevel) { $_.Principal.RunLevel } else { 'n/a' }; "
        "\"{0}`t{1}`t{2}`t{3}`t{4}\" -f $_.TaskPath,$_.TaskName,$_.State,$level,$actions }");

    AuditItem customTasks;
    customTasks.category = "Задачи";
    customTasks.title = "Активные не-Microsoft scheduled tasks";

    AuditItem suspiciousTasks;
    suspiciousTasks.category = "Задачи";
    suspiciousTasks.title = "Задачи с рискованными путями запуска";

    if (!result.success) {
        customTasks.status = AuditStatus::error;
        customTasks.details = {"Не удалось получить scheduled tasks: " + trim(result.errorMessage)};
        customTasks.recommendation = "Проверь доступность модуля ScheduledTasks в PowerShell.";
        suspiciousTasks.status = AuditStatus::info;
        suspiciousTasks.details = {"Пропущено, потому что перечисление задач завершилось ошибкой."};
        items.push_back(std::move(customTasks));
        items.push_back(std::move(suspiciousTasks));
        return items;
    }

    std::size_t customCount = 0;
    std::size_t suspiciousCount = 0;
    const std::vector<std::string> riskyMarkers {
        "\\appdata\\",
        "\\temp\\",
        "\\downloads\\",
        "\\users\\public\\",
    };

    for (const auto& line : splitLines(result.output)) {
        if (trim(line).empty()) {
            continue;
        }

        const auto fields = split(line, '\t');
        if (fields.size() < 5) {
            continue;
        }

        const auto path = trim(fields[0]);
        const auto name = trim(fields[1]);
        const auto state = trim(fields[2]);
        const auto runLevel = trim(fields[3]);
        const auto actions = trim(fields[4]);
        const auto descriptor = path + name + " :: state=" + state + ", runLevel=" + runLevel + ", actions=" + actions;

        if (state != "Disabled" && path.rfind("\\Microsoft\\", 0) != 0) {
            ++customCount;
            if (customTasks.details.size() < 10) {
                customTasks.details.push_back(descriptor);
            }
        }

        std::string loweredActions = actions;
        std::transform(loweredActions.begin(), loweredActions.end(), loweredActions.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

        if (!actions.empty() && containsAny(loweredActions, riskyMarkers)) {
            ++suspiciousCount;
            suspiciousTasks.details.push_back(descriptor);
        }
    }

    customTasks.status = customCount > 5 ? AuditStatus::warning : AuditStatus::ok;
    customTasks.recommendation = customCount > 5
        ? "Проверь сторонние и кастомные задачи планировщика: все ли они еще нужны."
        : "Количество активных не-Microsoft задач относительно небольшое.";
    if (customTasks.details.empty()) {
        customTasks.details.push_back("Активные не-Microsoft задачи не выделены.");
    }

    suspiciousTasks.status = suspiciousCount > 0 ? AuditStatus::warning : AuditStatus::ok;
    suspiciousTasks.recommendation = suspiciousCount > 0
        ? "Задачи, запускающие исполняемые файлы из user-writable путей, требуют ручной проверки."
        : "В выбранном наборе рискованных путей подозрительных action-path не найдено.";
    if (suspiciousTasks.details.empty()) {
        suspiciousTasks.details.push_back("Подозрительные пути запуска задач не найдены.");
    }

    items.push_back(std::move(customTasks));
    items.push_back(std::move(suspiciousTasks));
    return items;
}

}  // namespace sec
