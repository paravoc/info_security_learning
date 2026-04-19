#include "modules/commands/CommandCatalog.h"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace {

sec::CommandReference makeCommand(std::string platform,
                                  std::string category,
                                  std::string title,
                                  std::string command,
                                  std::string purpose,
                                  std::string caution,
                                  std::string exampleOutput,
                                  std::string interpretation,
                                  std::string relatedTool,
                                  std::string scenario,
                                  std::initializer_list<std::string> checks,
                                  std::initializer_list<std::string> nextCommands,
                                  const bool runnableInsideApp) {
    sec::CommandReference reference;
    reference.platform = std::move(platform);
    reference.category = std::move(category);
    reference.title = std::move(title);
    reference.command = std::move(command);
    reference.purpose = std::move(purpose);
    reference.caution = std::move(caution);
    reference.exampleOutput = std::move(exampleOutput);
    reference.interpretation = std::move(interpretation);
    reference.relatedTool = std::move(relatedTool);
    reference.scenario = std::move(scenario);
    reference.checks = checks;
    reference.nextCommands = nextCommands;
    reference.runnableInsideApp = runnableInsideApp;
    return reference;
}

}  // namespace

namespace sec {

std::vector<CommandReference> CommandCatalog::build() const {
    return {
        makeCommand(
            "Windows",
            "Учётные записи и доступ",
            "Локальные пользователи",
            "Get-LocalUser",
            "Показывает локальные учетные записи Windows, их состояние и базовые свойства.",
            "Read-only команда. Она нужна для аудита и инвентаризации, а не для доступа к чужим секретам.",
            R"(Name               Enabled LastLogon
----               ------- ---------
Administrator      False
DefaultAccount     False
student            True    19.04.2026 00:41:12
support            True    18.04.2026 21:10:03)",
            "Смотри, какие аккаунты включены, кто давно не входил и нет ли тестовых или сервисных учеток без понятной роли.",
            "Win+X -> Управление компьютером",
            "Когда нужно понять, кто вообще существует на локальной машине и какие учетные записи живые.",
            {"включен ли аккаунт", "дата LastLogon", "неожиданные локальные пользователи"},
            {"Get-LocalGroupMember -Group Administrators", "net accounts"},
            true),
        makeCommand(
            "Windows",
            "Учётные записи и доступ",
            "Парольная политика",
            "net accounts",
            "Показывает локальную парольную политику: длину, срок действия, историю и параметры блокировки.",
            "На доменных машинах часть настроек может приходить из GPO. Это быстрый первый срез, а не полный аудит домена.",
            R"(Minimum password age (days):                          0
Maximum password age (days):                          42
Minimum password length:                              8
Length of password history maintained:                5
Lockout threshold:                                    5)",
            "Если длина пароля маленькая, история нулевая и нет lockout threshold, локальная защита явно слабая.",
            "Win+X -> Терминал (Администратор)",
            "Когда нужно быстро понять базовый security baseline локальной системы.",
            {"Minimum password length", "history maintained", "Lockout threshold"},
            {"Get-LocalUser", "Get-LocalGroupMember -Group Administrators"},
            true),
        makeCommand(
            "Windows",
            "Учётные записи и доступ",
            "Локальные администраторы",
            "Get-LocalGroupMember -Group Administrators",
            "Показывает участников локальной группы администраторов.",
            "Лишний локальный администратор увеличивает риск ошибок и тихих изменений в системе.",
            R"(ObjectClass Name                         PrincipalSource
----------- ----                         ---------------
User        DESKTOP-01\student           Local
Group       DESKTOP-01\IT-Support        Local
User        CONTOSO\svc_backup           ActiveDirectory)",
            "Сравни список с реальной ролью хоста. Любой непонятный сервисный или пользовательский аккаунт здесь требует объяснения.",
            "Win+X -> Управление компьютером",
            "Когда проверяешь доступы, эскалацию прав и локальный administrative footprint.",
            {"лишние локальные админы", "сервисные учетки", "доменные учетные записи с admin-правами"},
            {"whoami /priv", "Get-LocalUser"},
            true),
        makeCommand(
            "Windows",
            "Учётные записи и доступ",
            "Текущие привилегии",
            "whoami /priv",
            "Показывает привилегии текущего токена: что реально выдано процессу и что сейчас активно.",
            "Команда не меняет права. Но она помогает не гадать, почему команда требует elevation.",
            R"(Privilege Name                Description                    State
==============                ===========                    =====
SeShutdownPrivilege           Shut down the system           Disabled
SeChangeNotifyPrivilege       Bypass traverse checking       Enabled
SeTimeZonePrivilege           Change the time zone           Disabled)",
            "Наличие привилегии не значит, что она используется. Смотри на столбец State и понимай, нужен ли тебе elevation.",
            "Win+X -> Терминал / Терминал (Администратор)",
            "Когда команда не работает и нужно понять, запущена ли сессия с нужным уровнем прав.",
            {"какие привилегии есть", "что Enabled", "что Disabled"},
            {"Get-LocalGroupMember -Group Administrators"},
            false),

        makeCommand(
            "Windows",
            "Сеть и DNS",
            "IP-конфигурация",
            "Get-NetIPConfiguration",
            "Показывает активные интерфейсы, IPv4/IPv6, шлюзы и DNS.",
            "Начинать диагностику сети лучше отсюда, а не с бессистемного изменения настроек.",
            R"(InterfaceAlias       : Ethernet
IPv4Address          : 192.168.1.34
IPv4DefaultGateway   : 192.168.1.1
DNSServer            : 192.168.1.1, 1.1.1.1

InterfaceAlias       : vEthernet (Default Switch)
IPv4Address          : 172.22.176.1)",
            "Отделяй реальный сетевой интерфейс от виртуальных switch, VPN и контейнерных адаптеров.",
            "Win+X -> Сетевые подключения",
            "Когда сайт не открывается или нужно понять, какой адрес машина получила на самом деле.",
            {"активный интерфейс", "gateway", "DNS", "виртуальные адаптеры"},
            {"Resolve-DnsName example.org", "Test-NetConnection example.org -Port 443"},
            true),
        makeCommand(
            "Windows",
            "Сеть и DNS",
            "DNS-резолвинг",
            "Resolve-DnsName example.org",
            "Проверяет, как система разрешает доменное имя.",
            "Это проверка диагностики. Не используй ее как сканер чужих доменов и инфраструктур.",
            R"(Name                                           Type   TTL   Section    IPAddress
----                                           ----   ---   -------    ---------
example.org                                    A      285   Answer     93.184.216.34)",
            "Если DNS не отдает запись, проблема еще до TCP-подключения. Если запись есть, переходи к проверке порта.",
            "Win+X -> Терминал",
            "Когда нужно понять, виноват ли DNS, а не браузер или удаленный веб-сервер.",
            {"есть ли Answer", "какой IP вернулся", "нет ли неожиданного DNS-ответа"},
            {"Test-NetConnection example.org -Port 443", "Get-NetIPConfiguration"},
            true),
        makeCommand(
            "Windows",
            "Сеть и DNS",
            "Проверка удалённого порта",
            "Test-NetConnection example.org -Port 443",
            "Проверяет DNS-резолвинг и TCP-доступность удаленной точки.",
            "Нормальная диагностическая команда. Не используй ее для агрессивного перебора хостов.",
            R"(ComputerName           : example.org
RemoteAddress          : 93.184.216.34
RemotePort             : 443
NameResolutionResults  : 93.184.216.34
TcpTestSucceeded       : True)",
            "Если имя резолвится, но TcpTestSucceeded=False, дальше проверяй firewall, маршрут или состояние удаленного сервиса.",
            "Win+X -> Терминал",
            "Когда нужно проверить, доступен ли сервис по конкретному TCP-порту.",
            {"TcpTestSucceeded", "RemoteAddress", "NameResolutionResults"},
            {"Get-NetIPConfiguration", "Get-NetTCPConnection -State Listen | Sort-Object LocalPort"},
            true),
        makeCommand(
            "Windows",
            "Сеть и DNS",
            "Слушающие TCP-порты",
            "Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess",
            "Показывает, какие локальные порты слушают входящие подключения.",
            "Особенно внимательно смотри на 3389, 445, 5985 и другие сервисные порты удаленного доступа.",
            R"(LocalAddress LocalPort OwningProcess
------------ --------- -------------
0.0.0.0      135       932
0.0.0.0      445       4
0.0.0.0      3389      1304
127.0.0.1    5354      4976)",
            "Важно не само наличие порта, а его смысл: кто слушает, на каком адресе и должен ли этот сервис быть доступен извне.",
            "Win+X -> Терминал (Администратор)",
            "Когда нужно понять, не торчит ли наружу лишний сервис или удаленный доступ.",
            {"0.0.0.0 против 127.0.0.1", "служебные порты", "OwningProcess"},
            {"Get-Service | Where-Object Status -eq 'Running'", "Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction"},
            true),

        makeCommand(
            "Windows",
            "Диски и файловые системы",
            "Свободное место на томах",
            "Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size",
            "Показывает свободное место на томах и помогает быстро найти переполненный раздел.",
            "Не пытайся чистить recovery или скрытые системные разделы без понимания их роли.",
            R"(DriveLetter FileSystemLabel SizeRemaining     Size
----------- --------------- -------------     ----
C           System          18.40 GB          237.93 GB
D           Data            412.11 GB         931.39 GB
            Recovery        152.00 MB         850.00 MB)",
            "Если системный диск близок к 100 процентам, проблемы пойдут в обновлениях, кэше, pagefile и временных файлах.",
            "Win+X -> Управление дисками",
            "Когда нужно быстро понять, где реально заканчивается место.",
            {"системный том", "скрытые разделы", "остаток на C:"},
            {"Get-PSDrive -PSProvider FileSystem", "Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime"},
            true),
        makeCommand(
            "Windows",
            "Диски и файловые системы",
            "Файловые диски PowerShell",
            "Get-PSDrive -PSProvider FileSystem",
            "Показывает файловые диски в удобном для PowerShell виде.",
            "Это быстрый рабочий срез. Для низкоуровневых проблем по дискам переходи к Get-Disk и оснастке управления дисками.",
            R"(Name Used (GB) Free (GB) Provider   Root
---- --------- --------- --------   ----
C      219.53     18.40 FileSystem C:\
D      519.28    412.11 FileSystem D:\)",
            "Удобно для ежедневной диагностики и скриптов. Видно только то, что реально смонтировано как файловая система.",
            "Win+X -> Управление дисками",
            "Когда нужен короткий срез по свободному месту без лишнего шума.",
            {"Used/Free", "только файловые системы", "сравнение с Get-Volume"},
            {"Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size", "Get-Disk | Select-Object Number,FriendlyName,HealthStatus,OperationalStatus,Size"},
            true),
        makeCommand(
            "Windows",
            "Диски и файловые системы",
            "Физические диски и health",
            "Get-Disk | Select-Object Number,FriendlyName,PartitionStyle,HealthStatus,OperationalStatus,Size",
            "Показывает физические диски, стиль разметки и базовый health-status.",
            "Если HealthStatus не в норме, не лечи это только очисткой: это уже вопрос состояния железа и storage-пути.",
            R"(Number FriendlyName       PartitionStyle HealthStatus OperationalStatus         Size
------ ------------       -------------- ------------ -----------------         ----
0      NVMe SSD 1TB       GPT            Healthy      Online            1000202273280
1      USB External Disk  MBR            Healthy      Online             500107862016)",
            "Команда полезна, чтобы не путать проблему файловой системы с проблемой самого устройства.",
            "Win+X -> Управление дисками",
            "Когда нужно понять, проблема в логическом томе или уже в самом диске.",
            {"HealthStatus", "OperationalStatus", "какой диск системный"},
            {"Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size"},
            true),
        makeCommand(
            "Windows",
            "Диски и файловые системы",
            "Содержимое TEMP",
            "Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime",
            "Показывает, что лежит в пользовательском TEMP и какие файлы там самые заметные.",
            "Не удаляй временные файлы наугад, если не понимаешь, кто их использует прямо сейчас.",
            R"(Name                         Length LastWriteTime
----                         ------ -------------
7zO12A.tmp                    84512 19.04.2026 00:54:12
ChromeProfileDump             4096  18.04.2026 22:11:44
Code Cache                    0     18.04.2026 21:07:12)",
            "Смотри на размер, возраст и повторяющиеся шаблоны. Это помогает понять, кто загрязняет TEMP.",
            "Параметры -> Система -> Память",
            "Когда нужно понять, что именно копится во временных каталогах перед очисткой.",
            {"старые файлы", "крупные объекты", "повторяющиеся кэши"},
            {"Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size"},
            false),

        makeCommand(
            "Windows",
            "Процессы и производительность",
            "Тяжёлые процессы по CPU",
            "Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS",
            "Помогает быстро найти процессы, которые съели больше всего процессорного времени.",
            "Высокий CPU не всегда означает проблему: индексация, обновления и антивирус могут давать нормальный фон.",
            R"(ProcessName        CPU      WS
-----------        ---      --
chrome             843.72   952320000
MsMpEng            312.61   284196864
Code               221.04   612794368
OneDrive            74.38   118550528)",
            "CPU показывает накопленное время. Ищи не просто большие числа, а процессы, которые долго растут без понятной причины.",
            "Win+X -> Диспетчер задач",
            "Когда система ощущается тяжелой и нужно быстро найти главного потребителя CPU.",
            {"долгоживущие процессы", "сочетание CPU и WS", "неожиданные имена"},
            {"Get-Counter '\\Memory\\Available MBytes'", "Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location"},
            true),
        makeCommand(
            "Windows",
            "Процессы и производительность",
            "Свободная память",
            "Get-Counter '\\Memory\\Available MBytes'",
            "Показывает, сколько памяти сейчас доступно системе.",
            "Одна метрика не заменяет полный memory analysis, но дает быстрый sanity-check.",
            R"CSV((PDH-CSV 4.0)","\\DESKTOP-01\memory\available mbytes"
"19.04.2026 01:20:12.312","914")CSV",
            "Если Available MBytes очень низок и держится таким долго, ищи приложение или сервис, который давит на память.",
            "Win+X -> Диспетчер задач",
            "Когда подозреваешь нехватку памяти, а не процессора.",
            {"длительно низкий available memory", "сопоставление с тяжелыми процессами"},
            {"Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS"},
            true),
        makeCommand(
            "Windows",
            "Процессы и производительность",
            "Автозагрузка",
            "Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location",
            "Показывает программы, стартующие вместе с Windows.",
            "Не путай полезные агенты синхронизации и защиты с реально лишним автозапуском.",
            R"(Name                Command                                              Location
----                -------                                              --------
OneDrive            "C:\Users\student\AppData\Local\Microsoft\..."        HKCU\Software\Microsoft\...
SecurityHealth      C:\Windows\system32\SecurityHealthSystray.exe         HKLM\Software\Microsoft\...
Teams Machine       C:\Program Files\Teams Installer\Teams.exe            Startup)",
            "Смотри на путь запуска. AppData, Temp и скрытые пользовательские папки требуют отдельной проверки.",
            "Win+X -> Диспетчер задач",
            "Когда логон медленный или хочется разобрать, что система запускает без спроса.",
            {"Location", "пути в AppData", "подписанные и штатные компоненты"},
            {"Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run','HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run'"},
            true),
        makeCommand(
            "Windows",
            "Процессы и производительность",
            "Отчёт по батарее",
            "powercfg /batteryreport",
            "Создает HTML-отчет по батарее ноутбука: емкость, циклы, историю использования.",
            "Команда создает файл-отчет. Практический смысл есть на ноутбуках и планшетах, а не на обычных стационарных ПК.",
            R"(Battery life report saved to file path
C:\Users\student\battery-report.html)",
            "Полезный результат находится не в консоли, а в HTML-файле. Смотри Design Capacity и Full Charge Capacity.",
            "Win+X -> Управление электропитанием",
            "Когда нужно понять, это тормоза из-за degraded battery/power profile или нет.",
            {"путь к HTML-отчету", "Design Capacity", "Full Charge Capacity"},
            {"powercfg /energy"},
            false),

        makeCommand(
            "Windows",
            "Службы и автозапуск",
            "Активные службы",
            "Get-Service | Where-Object Status -eq 'Running' | Select-Object -First 20 Name,DisplayName,Status",
            "Показывает работающие службы и помогает быстро понять профиль хоста.",
            "Не отключай службу вслепую: сначала пойми, системная она, защитная или прикладная.",
            R"(Name           DisplayName                               Status
----           -----------                               ------
LanmanServer   Server                                    Running
WinDefend      Microsoft Defender Antivirus Service      Running
wuauserv       Windows Update                            Running)",
            "Смотри на сетевые, защитные и удаленные сервисы. Неожиданное имя без понятной роли - уже повод копать дальше.",
            "Win+X -> Управление компьютером",
            "Когда нужно понять, что именно сейчас живет на хосте как сервис.",
            {"защитные сервисы", "сетевые службы", "службы удаленного доступа"},
            {"Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess"},
            true),
        makeCommand(
            "Windows",
            "Службы и автозапуск",
            "Активные задачи планировщика",
            "Get-ScheduledTask | Where-Object State -ne 'Disabled' | Select-Object -First 20 TaskName,TaskPath,State",
            "Показывает включенные scheduled tasks и помогает искать подозрительный автозапуск.",
            "Перед отключением задачи пойми, не отвечает ли она за драйвер, резервную копию или обновления.",
            R"(TaskName                         TaskPath                 State
--------                         --------                 -----
GoogleUpdaterTaskMachineCore     \                        Ready
MicrosoftEdgeUpdateTaskMachineUA \                        Ready
OneDrive Standalone Update Task  \                        Running)",
            "Особенно внимательно смотри на задачи из AppData, Temp и user-writable каталогов.",
            "Win+X -> Управление компьютером / taskschd.msc",
            "Когда автозапуск неочевиден и нужно проверить не только Run keys.",
            {"TaskPath", "Running/Ready", "пути к действиям задачи"},
            {"Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location"},
            true),
        makeCommand(
            "Windows",
            "Службы и автозапуск",
            "Run keys",
            "Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run','HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run'",
            "Показывает классические точки автозапуска из реестра.",
            "Особенно внимательно смотри пути из AppData, Temp, Downloads и нестандартных пользовательских каталогов.",
            R"(SecurityHealth   : C:\Windows\system32\SecurityHealthSystray.exe
OneDrive         : C:\Users\student\AppData\Local\Microsoft\OneDrive\OneDrive.exe
Updater          : C:\Users\student\AppData\Roaming\Vendor\updater.exe)",
            "Оцени не только имя записи, но и фактический путь к файлу. User-writable путь в автозапуске почти всегда требует разбора.",
            "Win+X -> Терминал (Администратор)",
            "Когда нужно добить ревизию автозагрузки после просмотра Task Scheduler и StartupCommand.",
            {"пути в AppData и Temp", "нештатные названия", "HKCU против HKLM"},
            {"Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location"},
            true),

        makeCommand(
            "Windows",
            "Журналы и защита",
            "Состояние Defender",
            "Get-MpComputerStatus",
            "Показывает состояние Microsoft Defender, сигнатур и real-time protection.",
            "Если Defender выключен, сначала проверь, не стоит ли другой endpoint protection. Не делай выводы по одному флагу.",
            R"(AMServiceEnabled          : True
AntivirusEnabled          : True
BehaviorMonitorEnabled    : True
RealTimeProtectionEnabled : True
QuickScanAge              : 1)",
            "Тебя интересуют флаги Enabled, свежесть сигнатур и отключенные защитные компоненты.",
            "Безопасность Windows / Терминал (Администратор)",
            "Когда нужно быстро понять, жива ли базовая защита на хосте.",
            {"RealTimeProtectionEnabled", "AntivirusEnabled", "возраст сигнатур"},
            {"Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction"},
            true),
        makeCommand(
            "Windows",
            "Журналы и защита",
            "Профили firewall",
            "Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction",
            "Показывает, включены ли профили Domain, Private и Public и как они обрабатывают трафик.",
            "Отключенный Public profile или слишком разрешающий inbound-профиль требует отдельного объяснения.",
            R"(Name    Enabled DefaultInboundAction DefaultOutboundAction
----    ------- -------------------- ---------------------
Domain  True    NotConfigured        NotConfigured
Private True    Block                Allow
Public  True    Block                Allow)",
            "Для рабочей станции типичный baseline: Public и Private включены, входящие подключения по умолчанию блокируются.",
            "Win+X -> Терминал (Администратор)",
            "Когда проверяешь exposed services и общую сетевую гигиену хоста.",
            {"Enabled", "DefaultInboundAction", "Public profile"},
            {"Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess"},
            true),
        makeCommand(
            "Windows",
            "Журналы и защита",
            "Последние системные события",
            "Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message",
            "Быстро показывает свежие события системного журнала Windows.",
            "Системный журнал шумный. Ищи повторяющегося провайдера и повторяющиеся ошибки, а не случайный единичный event.",
            R"(TimeCreated          Id ProviderName                   LevelDisplayName Message
-----------          -- ------------                   ---------------- -------
19.04.2026 01:05:22 7036 Service Control Manager       Information      The Windows Update service entered the running state.
19.04.2026 01:04:11 10016 DistributedCOM               Warning          The application-specific permission settings do not grant Local Activation...
19.04.2026 01:03:01 6005 EventLog                      Information      The Event log service was started.)",
            "Это команда для поиска паттерна: один и тот же provider, один и тот же ID, одна и та же история ошибок.",
            "Win+X -> Просмотр событий",
            "Когда машина ругается, но еще неясно, смотреть в сеть, службы или storage.",
            {"повторяющийся ProviderName", "LevelDisplayName=Error/Warning", "сообщения рядом по времени"},
            {"Get-Service | Where-Object Status -eq 'Running' | Select-Object -First 20 Name,DisplayName,Status"},
            true),
        makeCommand(
            "Windows",
            "Журналы и защита",
            "Свежие обновления",
            "Get-HotFix | Sort-Object InstalledOn -Descending | Select-Object -First 10 HotFixID,InstalledOn,Description",
            "Показывает последние установленные hotfix и обновления.",
            "Это не полная картина Windows Update, но для первого прохода очень полезно.",
            R"(HotFixID   InstalledOn Description
--------   ----------- -----------
KB5036893  18.04.2026  Update
KB5035702  18.04.2026  Security Update
KB5035155  10.04.2026  Update)",
            "Полезно видеть, обновлялась ли машина недавно и не застряла ли она на древнем уровне патчей.",
            "Параметры -> Центр обновления Windows",
            "Когда хочешь понять, насколько актуальна система по патчам.",
            {"InstalledOn", "security updates", "давно ли не обновлялась машина"},
            {"Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message"},
            true),

        makeCommand(
            "Linux",
            "Доступ и учётные записи",
            "Текущий пользователь и группы",
            "id",
            "Показывает текущего пользователя, uid, gid и группы.",
            "Это базовая команда чтения. На сервере она помогает сразу понять, работаешь ли ты под нужной учеткой.",
            R"(uid=1000(student) gid=1000(student) groups=1000(student),4(adm),27(sudo),999(docker))",
            "Лишняя группа sudo, wheel, docker или adm уже меняет картину риска и видимости системы.",
            "id / groups / sudo",
            "Когда нужно понять, под кем ты реально работаешь и какие группы расширяют твои возможности.",
            {"наличие sudo/wheel/docker", "uid/gid", "лишние группы"},
            {"sudo -l", "last -a | head -n 10"},
            true),
        makeCommand(
            "Linux",
            "Доступ и учётные записи",
            "Локальные учётные записи",
            "getent passwd | head -n 10",
            "Показывает первые локальные записи из passwd и напоминает, как выглядит структура пользователей.",
            "На production-хостах полный список обычно длиннее. Здесь полезен сам формат и первичная инвентаризация.",
            R"(root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
student:x:1000:1000:Student:/home/student:/bin/bash)",
            "Смотри на shell, uid и домашний каталог. `/usr/sbin/nologin` и `/bin/false` отделяют сервисные учетные записи от интерактивных.",
            "getent / /etc/passwd",
            "Когда нужно понять, какие локальные пользователи есть и кто из них интерактивный.",
            {"shell пользователя", "uid<1000 против uid>=1000", "домашний каталог"},
            {"id", "last -a | head -n 10"},
            true),
        makeCommand(
            "Linux",
            "Доступ и учётные записи",
            "Права sudo",
            "sudo -l",
            "Показывает, какие команды текущий пользователь может запускать через sudo.",
            "Команда часто требует пароль и дает чувствительную информацию о правах. Используй ее только на своих системах и в рамках аудита.",
            R"(Matching Defaults entries for student on lab:
    env_reset, mail_badpass, secure_path=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

User student may run the following commands on lab:
    (ALL : ALL) ALL)",
            "Если видишь ALL или NOPASSWD на широкий набор команд, это очень сильные привилегии.",
            "sudoers / /etc/sudoers.d",
            "Когда нужно понять, ограничены ли права пользователя или он фактически администратор.",
            {"NOPASSWD", "ALL : ALL", "узко ограниченные команды"},
            {"id", "last -a | head -n 10"},
            false),
        makeCommand(
            "Linux",
            "Доступ и учётные записи",
            "История входов",
            "last -a | head -n 10",
            "Показывает последние интерактивные входы и источники подключений.",
            "На контейнерах и свежих лабораториях вывод может быть пустым. Это не ошибка, а особенность среды.",
            R"(student  pts/0        192.168.1.20     Fri Apr 19 00:51   still logged in
reboot   system boot  6.6.36.6-microsoft Fri Apr 19 00:48)",
            "Ищи неожиданные внешние IP и непривычные окна времени. Потом связывай это с SSH-логами.",
            "last / journalctl -u ssh",
            "Когда нужно быстро посмотреть, были ли интерактивные входы и откуда.",
            {"IP-адрес источника", "время входа", "still logged in"},
            {"sudo journalctl -u ssh -n 20", "id"},
            true),

        makeCommand(
            "Linux",
            "Сеть и DNS",
            "Сетевые интерфейсы",
            "ip addr show",
            "Показывает сетевые интерфейсы Linux и назначенные адреса.",
            "Это современная замена ifconfig на большинстве дистрибутивов.",
            R"(2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500
    inet 192.168.1.52/24 brd 192.168.1.255 scope global eth0
3: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500
    inet 172.17.0.1/16 scope global docker0)",
            "Отделяй основной интерфейс от виртуальных мостов Docker, VPN и hypervisor-сетей.",
            "ip / NetworkManager / systemd-networkd",
            "Когда сервис недоступен и сначала нужно понять, жив ли вообще сетевой стек хоста.",
            {"основной интерфейс", "IPv4/IPv6", "виртуальные интерфейсы"},
            {"ip route", "ss -tulpn"},
            true),
        makeCommand(
            "Linux",
            "Сеть и DNS",
            "Маршруты",
            "ip route",
            "Показывает таблицу маршрутизации Linux.",
            "Без маршрута по умолчанию выход наружу невозможен, как бы хорошо ни выглядел интерфейс.",
            R"(default via 192.168.1.1 dev eth0 proto dhcp src 192.168.1.52 metric 100
172.17.0.0/16 dev docker0 proto kernel scope link src 172.17.0.1)",
            "Если нет default route, проблема уже на уровне сетевой конфигурации или DHCP, а не у приложения.",
            "ip / routing",
            "Когда хост не видит внешние адреса или странно идет только локальный трафик.",
            {"default via", "какой dev используется", "виртуальные подсети"},
            {"ip addr show", "getent hosts example.org"},
            true),
        makeCommand(
            "Linux",
            "Сеть и DNS",
            "Проверка DNS",
            "getent hosts example.org",
            "Проверяет, как система резолвит имя через настроенные механизмы NSS/DNS.",
            "Это удобнее для базовой проверки, чем сразу лезть в dig, особенно на минимальных системах.",
            R"(93.184.216.34   example.org)",
            "Если имя не резолвится, дальше нет смысла проверять TCP-порт до исправления DNS.",
            "getent / resolv.conf",
            "Когда нужно отделить проблему резолвинга от проблемы соединения.",
            {"есть ли IP в ответе", "неожиданный адрес", "вообще есть ли ответ"},
            {"ss -tulpn", "ip route"},
            true),
        makeCommand(
            "Linux",
            "Сеть и DNS",
            "Слушающие порты",
            "ss -tulpn",
            "Показывает TCP и UDP listeners вместе с owning process.",
            "На production-хостах лучше фильтровать вывод по конкретному порту, чтобы не тонуть в шуме.",
            R"(Netid State  Local Address:Port Process
tcp   LISTEN 0.0.0.0:22       users:(("sshd",pid=728,fd=3))
tcp   LISTEN 127.0.0.1:5432   users:(("postgres",pid=991,fd=5))
tcp   LISTEN 0.0.0.0:8080     users:(("java",pid=2044,fd=84)))",
            "Принцип тот же, что и на Windows: кто слушает, на каком адресе и должен ли этот сервис торчать наружу.",
            "ss / systemctl / journalctl",
            "Когда нужно быстро сопоставить сервис, порт и процесс.",
            {"0.0.0.0 против 127.0.0.1", "неожиданные порты", "имя процесса"},
            {"systemctl --type=service --state=running --no-pager | head -n 20"},
            true),

        makeCommand(
            "Linux",
            "Диски и файловые системы",
            "Свободное место на файловых системах",
            "df -h",
            "Показывает заполнение файловых систем в удобочитаемом формате.",
            "Не путай обычные тома с tmpfs и контейнерными mounts.",
            R"(Filesystem      Size  Used Avail Use% Mounted on
/dev/sda2        80G   62G   14G  82% /
tmpfs           3.8G  1.2M  3.8G   1% /run
/dev/sdb1       200G   91G  100G  48% /data)",
            "Если системный раздел перевалил за 80-90 процентов, пора смотреть логи, кэши и контейнерные данные.",
            "df / storage",
            "Когда нужно быстро понять, какой mount point на грани.",
            {"Use%", "корневой раздел", "tmpfs отдельно от обычных дисков"},
            {"lsblk -f", "findmnt"},
            true),
        makeCommand(
            "Linux",
            "Диски и файловые системы",
            "Блочные устройства",
            "lsblk -f",
            "Показывает блочные устройства, файловые системы, UUID и точки монтирования.",
            "Это карта хранилища. Она нужна, чтобы не перепутать физический диск, раздел и точку монтирования.",
            R"(NAME   FSTYPE LABEL UUID                                 MOUNTPOINT
sda
├─sda1 ext4         2b99b1d8-2a1a-4f9c-8bd3-a9d1d1c2a900 /boot
└─sda2 ext4 root    d551a8db-4518-4f4d-b91c-87f5a0af1100 /
sdb1 xfs  data      8f84b0b9-00e9-430d-8d5d-30dd1aef96b1 /data)",
            "Команда помогает быстро увидеть, где root, где data и что реально смонтировано.",
            "lsblk / findmnt",
            "Когда нужно понять топологию дисков и разделов, а не только проценты заполнения.",
            {"FSTYPE", "MOUNTPOINT", "какое устройство системное"},
            {"df -h", "findmnt"},
            true),
        makeCommand(
            "Linux",
            "Диски и файловые системы",
            "Точки монтирования",
            "findmnt",
            "Показывает дерево точек монтирования и реальные source/target пары.",
            "Очень полезно на системах с контейнерами, overlayfs и большим числом mount points.",
            R"(TARGET SOURCE           FSTYPE OPTIONS
/      /dev/sda2        ext4   rw,relatime
/boot  /dev/sda1        ext4   rw,relatime
/data  /dev/sdb1        xfs    rw,relatime)",
            "Команда хороша, когда нужно не просто знать размер, а понимать, откуда и куда смонтирован каждый том.",
            "findmnt / mount",
            "Когда путаница именно в mount points, а не в общем объеме диска.",
            {"TARGET", "SOURCE", "overlay/tmpfs"},
            {"df -h", "lsblk -f"},
            true),

        makeCommand(
            "Linux",
            "Процессы и сервисы",
            "Тяжёлые процессы по памяти",
            "ps aux --sort=-%mem | head -n 10",
            "Показывает процессы, которые занимают больше всего памяти.",
            "Для live-наблюдения удобнее top/htop, но этот срез хорош для отчетов и заметок.",
            R"(USER   PID %CPU %MEM COMMAND
mysql  944  5.1 28.4 /usr/sbin/mysqld
java  2011  9.7 14.2 /usr/bin/java -jar app.jar
root   728  0.0  0.6 sshd: /usr/sbin/sshd -D)",
            "Смотри не только на %MEM, но и на роль процесса. База данных может есть память штатно, а случайный java/node-процесс - уже повод разбираться.",
            "ps / top / htop",
            "Когда подозреваешь memory pressure и нужно найти главного потребителя.",
            {"%MEM", "роль процесса", "неожиданные демоны"},
            {"ps aux --sort=-%cpu | head -n 10", "systemctl --type=service --state=running --no-pager | head -n 20"},
            true),
        makeCommand(
            "Linux",
            "Процессы и сервисы",
            "Тяжёлые процессы по CPU",
            "ps aux --sort=-%cpu | head -n 10",
            "Показывает процессы, которые потребляют больше всего CPU.",
            "Один короткий всплеск не всегда проблема. Ищи устойчивую нагрузку и повторяемый pattern.",
            R"(USER   PID %CPU %MEM COMMAND
java  2011 79.3 14.2 /usr/bin/java -jar app.jar
root  1448 12.8  1.1 /usr/bin/containerd
mysql  944  5.1 28.4 /usr/sbin/mysqld)",
            "CPU сам по себе мало о чем говорит, если не понимаешь роль процесса и время наблюдения.",
            "ps / top / htop",
            "Когда хост тормозит и нужно быстро локализовать потребителя CPU.",
            {"%CPU", "повторяющиеся процессы", "службы без понятной роли"},
            {"systemctl --type=service --state=running --no-pager | head -n 20", "ss -tulpn"},
            true),
        makeCommand(
            "Linux",
            "Процессы и сервисы",
            "Активные systemd-сервисы",
            "systemctl --type=service --state=running --no-pager | head -n 20",
            "Показывает работающие systemd-сервисы и помогает понять профиль машины.",
            "На минимальных образах или контейнерах systemd может быть не основным init, и команда будет менее полезна.",
            R"(ssh.service          loaded active running OpenBSD Secure Shell server
cron.service         loaded active running Regular background program processing daemon
docker.service       loaded active running Docker Application Container Engine)",
            "По списку можно быстро понять роль машины: jump-host, application server, container node и так далее.",
            "systemctl / journalctl",
            "Когда нужно понять, какие демоны реально запущены, а не просто установлены.",
            {"unexpected services", "роль хоста", "сетевые/удаленные сервисы"},
            {"ss -tulpn", "ps aux --sort=-%mem | head -n 10"},
            true),
        makeCommand(
            "Linux",
            "Процессы и сервисы",
            "Нагрузка системы",
            "uptime",
            "Показывает время работы системы и load average.",
            "Load average нужно читать вместе с числом CPU и типом нагрузки, а не как абстрактную страшную цифру.",
            R"( 01:42:18 up 2 days,  4:22,  2 users,  load average: 0.42, 0.51, 0.56)",
            "Стабильно высокий load без понятной причины - повод перейти к top/ps и смотреть, кто грузит систему.",
            "uptime / top",
            "Когда нужен очень быстрый health-check по нагрузке хоста.",
            {"load average 1/5/15", "время работы", "число пользователей"},
            {"ps aux --sort=-%cpu | head -n 10"},
            true),

        makeCommand(
            "Linux",
            "Журналы и безопасность",
            "Критичные события журнала",
            "journalctl -p 3 -xb | tail -n 20",
            "Показывает ошибки текущей загрузки системы из journalctl.",
            "На некоторых системах нужен sudo или доступ к группе systemd-journal.",
            R"(Apr 19 00:58:10 lab kernel: EXT4-fs warning (device sda2): ext4_end_bio:344: I/O error 10 writing to inode 524312
Apr 19 00:58:12 lab sshd[742]: error: maximum authentication attempts exceeded for invalid user admin)",
            "Ищи не единичную строку, а повторяющийся тип ошибки: storage, auth, network, service startup.",
            "journalctl",
            "Когда нужно быстро понять, о чем система чаще всего жалуется прямо сейчас.",
            {"повторяющиеся ошибки", "storage/auth/network", "текущая загрузка"},
            {"systemctl --type=service --state=running --no-pager | head -n 20"},
            false),
        makeCommand(
            "Linux",
            "Журналы и безопасность",
            "SSH-журнал",
            "sudo journalctl -u ssh -n 20",
            "Показывает последние события SSH: логины, ошибки и brute-force попытки.",
            "Обычно нужны повышенные права. Не запускай бездумно на чужих системах.",
            R"(Apr 19 00:51:02 lab sshd[742]: Accepted password for student from 192.168.1.20 port 50122 ssh2
Apr 19 00:53:11 lab sshd[756]: Failed password for invalid user admin from 185.22.44.9 port 45118 ssh2)",
            "Успешные и неуспешные входы нужно читать вместе: кто стучался, откуда и был ли потом успешный логин тем же источником.",
            "journalctl / auth.log",
            "Когда анализируешь удаленный доступ и подозреваешь перебор или несанкционированный вход.",
            {"Accepted/Failed password", "IP источника", "повторяющийся brute-force"},
            {"last -a | head -n 10", "id"},
            false),
    };
}

}  // namespace sec
