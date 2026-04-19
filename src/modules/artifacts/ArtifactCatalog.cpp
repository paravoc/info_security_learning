#include "modules/artifacts/ArtifactCatalog.h"

namespace sec {

std::vector<ArtifactLocation> ArtifactCatalog::build() const {
    return {
        {
            "Windows",
            "Учетные записи и доступ",
            "Локальные пользователи и группы",
            {
                "lusrmgr.msc",
                "Get-LocalUser",
                "Get-LocalGroupMember -Group Administrators",
            },
            "Get-LocalUser | Format-Table Name,Enabled,LastLogon",
            "Здесь проверяют локальные учетные записи, состояние аккаунтов и состав администраторов."
        },
        {
            "Windows",
            "Журналы и события",
            "Журнал безопасности Windows",
            {
                "Просмотр событий -> Windows Logs -> Security",
                "wevtutil el",
                "Get-WinEvent -LogName Security",
            },
            "Get-WinEvent -LogName Security -MaxEvents 20",
            "Журнал Security нужен для разбора входов, блокировок, смены привилегий и другой активности безопасности."
        },
        {
            "Windows",
            "Сеть и удаленный доступ",
            "Профили и лог межсетевого экрана",
            {
                "Get-NetFirewallProfile",
                "%systemroot%\\system32\\logfiles\\firewall\\pfirewall.log",
            },
            "Get-NetFirewallProfile; Get-Content $env:SystemRoot\\System32\\LogFiles\\Firewall\\pfirewall.log -Tail 50",
            "Здесь видно, включены ли профили firewall и что именно журналируется по сети."
        },
        {
            "Windows",
            "Автозагрузка и запуск",
            "Run keys и автозапуск пользователя",
            {
                "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                "%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup",
            },
            "Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run','HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run'",
            "Отсюда удобно проверять классические точки автозапуска и нежелательные пользовательские записи."
        },
        {
            "Windows",
            "Службы и планировщик",
            "Службы Windows",
            {
                "services.msc",
                "HKLM\\SYSTEM\\CurrentControlSet\\Services",
                "Get-Service",
            },
            "Get-Service | Sort-Object Status,DisplayName",
            "Здесь смотрят состояние служб, тип запуска и неожиданные сервисы удаленного доступа."
        },
        {
            "Windows",
            "Службы и планировщик",
            "Планировщик задач",
            {
                "taskschd.msc",
                "C:\\Windows\\System32\\Tasks",
                "Get-ScheduledTask",
            },
            "Get-ScheduledTask | Where-Object State -ne 'Disabled'",
            "Полезно для поиска кастомных задач, скрытой автоматизации и нежелательных путей запуска."
        },
        {
            "Windows",
            "PowerShell и администрирование",
            "История PowerShell",
            {
                "%APPDATA%\\Microsoft\\Windows\\PowerShell\\PSReadLine\\ConsoleHost_history.txt",
                "%USERPROFILE%\\Documents\\WindowsPowerShell",
            },
            "Get-Content (Join-Path $env:APPDATA 'Microsoft\\Windows\\PowerShell\\PSReadLine\\ConsoleHost_history.txt') -Tail 50",
            "История команд помогает понять, что запускалось на машине, но смотреть ее нужно только в рамках легитимной диагностики."
        },
        {
            "Windows",
            "Временные данные и кэш",
            "Temp и корзина",
            {
                "%TEMP%",
                "C:\\Windows\\Temp",
                "C:\\$Recycle.Bin",
            },
            "Get-ChildItem $env:TEMP -Force | Select-Object -First 20",
            "Здесь находятся временные файлы, пользовательский мусор и безопасные цели для controlled cleanup."
        },
        {
            "Linux",
            "Журналы и аутентификация",
            "Аутентификация и SSH",
            {
                "/var/log/auth.log",
                "journalctl -u ssh",
                "/var/log/secure",
            },
            "sudo journalctl -u ssh -n 50",
            "Эти источники используют для проверки входов, brute-force и проблем с удаленным доступом."
        },
        {
            "Linux",
            "Службы и автозапуск",
            "systemd и cron",
            {
                "/etc/systemd/system",
                "/usr/lib/systemd/system",
                "/etc/crontab",
                "/etc/cron.d",
                "crontab -l",
            },
            "systemctl list-unit-files --type=service; crontab -l",
            "Здесь видны системные сервисы, кастомные unit-файлы и регулярные задания."
        },
        {
            "Linux",
            "Сеть и маршрутизация",
            "Сетевые настройки и резолвинг",
            {
                "ip addr show",
                "ip route show",
                "/etc/hosts",
                "/etc/resolv.conf",
            },
            "ip addr show && ip route show",
            "Этот набор помогает быстро понять адреса, маршруты, DNS и локальные переопределения имен."
        },
        {
            "Linux",
            "Права и пользователи",
            "Пользователи, sudo и sudoers",
            {
                "/etc/passwd",
                "/etc/group",
                "/etc/sudoers",
                "/etc/sudoers.d",
            },
            "getent passwd | cut -d: -f1,7; sudo -l",
            "Здесь проверяют интерактивные аккаунты, оболочки и правила привилегированного доступа."
        },
        {
            "Linux",
            "Следы пользователя",
            "История shell и кэш",
            {
                "~/.bash_history",
                "~/.zsh_history",
                "~/.cache",
                "/tmp",
                "/var/tmp",
            },
            "ls -la ~/.bash_history ~/.zsh_history ~/.cache /tmp /var/tmp",
            "Полезно для ориентирования в пользовательской активности, временных файлах и безопасной очистке."
        },
    };
}

}  // namespace sec
