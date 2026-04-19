# Windows Security Workbench

Сформировано: 2026-04-19 18:39:24

Сценарий: practice

## Команды и шпаргалки

### [Windows] Учётные записи и доступ

#### Локальные пользователи

```powershell
Get-LocalUser
```

- Связанная оснастка: Win+X -> Управление компьютером
- Когда использовать: Когда нужно понять, кто вообще существует на локальной машине и какие учетные записи живые.
- Назначение: Показывает локальные учетные записи Windows, их состояние и базовые свойства.
- Осторожно: Read-only команда. Она нужна для аудита и инвентаризации, а не для доступа к чужим секретам.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name               Enabled LastLogon
----               ------- ---------
Administrator      False
DefaultAccount     False
student            True    19.04.2026 00:41:12
support            True    18.04.2026 21:10:03
```

- Как читать результат: Смотри, какие аккаунты включены, кто давно не входил и нет ли тестовых или сервисных учеток без понятной роли.
- На что смотреть:
- включен ли аккаунт
- дата LastLogon
- неожиданные локальные пользователи
- Что проверить потом:
- Get-LocalGroupMember -Group Administrators
- net accounts

#### Парольная политика

```powershell
net accounts
```

- Связанная оснастка: Win+X -> Терминал (Администратор)
- Когда использовать: Когда нужно быстро понять базовый security baseline локальной системы.
- Назначение: Показывает локальную парольную политику: длину, срок действия, историю и параметры блокировки.
- Осторожно: На доменных машинах часть настроек может приходить из GPO. Это быстрый первый срез, а не полный аудит домена.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Minimum password age (days):                          0
Maximum password age (days):                          42
Minimum password length:                              8
Length of password history maintained:                5
Lockout threshold:                                    5
```

- Как читать результат: Если длина пароля маленькая, история нулевая и нет lockout threshold, локальная защита явно слабая.
- На что смотреть:
- Minimum password length
- history maintained
- Lockout threshold
- Что проверить потом:
- Get-LocalUser
- Get-LocalGroupMember -Group Administrators

#### Локальные администраторы

```powershell
Get-LocalGroupMember -Group Administrators
```

- Связанная оснастка: Win+X -> Управление компьютером
- Когда использовать: Когда проверяешь доступы, эскалацию прав и локальный administrative footprint.
- Назначение: Показывает участников локальной группы администраторов.
- Осторожно: Лишний локальный администратор увеличивает риск ошибок и тихих изменений в системе.
- Можно запускать внутри приложения: да

Пример вывода:

```text
ObjectClass Name                         PrincipalSource
----------- ----                         ---------------
User        DESKTOP-01\student           Local
Group       DESKTOP-01\IT-Support        Local
User        CONTOSO\svc_backup           ActiveDirectory
```

- Как читать результат: Сравни список с реальной ролью хоста. Любой непонятный сервисный или пользовательский аккаунт здесь требует объяснения.
- На что смотреть:
- лишние локальные админы
- сервисные учетки
- доменные учетные записи с admin-правами
- Что проверить потом:
- whoami /priv
- Get-LocalUser

#### Текущие привилегии

```powershell
whoami /priv
```

- Связанная оснастка: Win+X -> Терминал / Терминал (Администратор)
- Когда использовать: Когда команда не работает и нужно понять, запущена ли сессия с нужным уровнем прав.
- Назначение: Показывает привилегии текущего токена: что реально выдано процессу и что сейчас активно.
- Осторожно: Команда не меняет права. Но она помогает не гадать, почему команда требует elevation.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Privilege Name                Description                    State
==============                ===========                    =====
SeShutdownPrivilege           Shut down the system           Disabled
SeChangeNotifyPrivilege       Bypass traverse checking       Enabled
SeTimeZonePrivilege           Change the time zone           Disabled
```

- Как читать результат: Наличие привилегии не значит, что она используется. Смотри на столбец State и понимай, нужен ли тебе elevation.
- На что смотреть:
- какие привилегии есть
- что Enabled
- что Disabled
- Что проверить потом:
- Get-LocalGroupMember -Group Administrators

### [Windows] Сеть и DNS

#### IP-конфигурация

```powershell
Get-NetIPConfiguration
```

- Связанная оснастка: Win+X -> Сетевые подключения
- Когда использовать: Когда сайт не открывается или нужно понять, какой адрес машина получила на самом деле.
- Назначение: Показывает активные интерфейсы, IPv4/IPv6, шлюзы и DNS.
- Осторожно: Начинать диагностику сети лучше отсюда, а не с бессистемного изменения настроек.
- Можно запускать внутри приложения: да

Пример вывода:

```text
InterfaceAlias       : Ethernet
IPv4Address          : 192.168.1.34
IPv4DefaultGateway   : 192.168.1.1
DNSServer            : 192.168.1.1, 1.1.1.1

InterfaceAlias       : vEthernet (Default Switch)
IPv4Address          : 172.22.176.1
```

- Как читать результат: Отделяй реальный сетевой интерфейс от виртуальных switch, VPN и контейнерных адаптеров.
- На что смотреть:
- активный интерфейс
- gateway
- DNS
- виртуальные адаптеры
- Что проверить потом:
- Resolve-DnsName example.org
- Test-NetConnection example.org -Port 443

#### DNS-резолвинг

```powershell
Resolve-DnsName example.org
```

- Связанная оснастка: Win+X -> Терминал
- Когда использовать: Когда нужно понять, виноват ли DNS, а не браузер или удаленный веб-сервер.
- Назначение: Проверяет, как система разрешает доменное имя.
- Осторожно: Это проверка диагностики. Не используй ее как сканер чужих доменов и инфраструктур.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name                                           Type   TTL   Section    IPAddress
----                                           ----   ---   -------    ---------
example.org                                    A      285   Answer     93.184.216.34
```

- Как читать результат: Если DNS не отдает запись, проблема еще до TCP-подключения. Если запись есть, переходи к проверке порта.
- На что смотреть:
- есть ли Answer
- какой IP вернулся
- нет ли неожиданного DNS-ответа
- Что проверить потом:
- Test-NetConnection example.org -Port 443
- Get-NetIPConfiguration

#### Проверка удалённого порта

```powershell
Test-NetConnection example.org -Port 443
```

- Связанная оснастка: Win+X -> Терминал
- Когда использовать: Когда нужно проверить, доступен ли сервис по конкретному TCP-порту.
- Назначение: Проверяет DNS-резолвинг и TCP-доступность удаленной точки.
- Осторожно: Нормальная диагностическая команда. Не используй ее для агрессивного перебора хостов.
- Можно запускать внутри приложения: да

Пример вывода:

```text
ComputerName           : example.org
RemoteAddress          : 93.184.216.34
RemotePort             : 443
NameResolutionResults  : 93.184.216.34
TcpTestSucceeded       : True
```

- Как читать результат: Если имя резолвится, но TcpTestSucceeded=False, дальше проверяй firewall, маршрут или состояние удаленного сервиса.
- На что смотреть:
- TcpTestSucceeded
- RemoteAddress
- NameResolutionResults
- Что проверить потом:
- Get-NetIPConfiguration
- Get-NetTCPConnection -State Listen | Sort-Object LocalPort

#### Слушающие TCP-порты

```powershell
Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess
```

- Связанная оснастка: Win+X -> Терминал (Администратор)
- Когда использовать: Когда нужно понять, не торчит ли наружу лишний сервис или удаленный доступ.
- Назначение: Показывает, какие локальные порты слушают входящие подключения.
- Осторожно: Особенно внимательно смотри на 3389, 445, 5985 и другие сервисные порты удаленного доступа.
- Можно запускать внутри приложения: да

Пример вывода:

```text
LocalAddress LocalPort OwningProcess
------------ --------- -------------
0.0.0.0      135       932
0.0.0.0      445       4
0.0.0.0      3389      1304
127.0.0.1    5354      4976
```

- Как читать результат: Важно не само наличие порта, а его смысл: кто слушает, на каком адресе и должен ли этот сервис быть доступен извне.
- На что смотреть:
- 0.0.0.0 против 127.0.0.1
- служебные порты
- OwningProcess
- Что проверить потом:
- Get-Service | Where-Object Status -eq 'Running'
- Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction

### [Windows] Диски и файловые системы

#### Свободное место на томах

```powershell
Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size
```

- Связанная оснастка: Win+X -> Управление дисками
- Когда использовать: Когда нужно быстро понять, где реально заканчивается место.
- Назначение: Показывает свободное место на томах и помогает быстро найти переполненный раздел.
- Осторожно: Не пытайся чистить recovery или скрытые системные разделы без понимания их роли.
- Можно запускать внутри приложения: да

Пример вывода:

```text
DriveLetter FileSystemLabel SizeRemaining     Size
----------- --------------- -------------     ----
C           System          18.40 GB          237.93 GB
D           Data            412.11 GB         931.39 GB
            Recovery        152.00 MB         850.00 MB
```

- Как читать результат: Если системный диск близок к 100 процентам, проблемы пойдут в обновлениях, кэше, pagefile и временных файлах.
- На что смотреть:
- системный том
- скрытые разделы
- остаток на C:
- Что проверить потом:
- Get-PSDrive -PSProvider FileSystem
- Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime

#### Файловые диски PowerShell

```powershell
Get-PSDrive -PSProvider FileSystem
```

- Связанная оснастка: Win+X -> Управление дисками
- Когда использовать: Когда нужен короткий срез по свободному месту без лишнего шума.
- Назначение: Показывает файловые диски в удобном для PowerShell виде.
- Осторожно: Это быстрый рабочий срез. Для низкоуровневых проблем по дискам переходи к Get-Disk и оснастке управления дисками.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name Used (GB) Free (GB) Provider   Root
---- --------- --------- --------   ----
C      219.53     18.40 FileSystem C:\
D      519.28    412.11 FileSystem D:\
```

- Как читать результат: Удобно для ежедневной диагностики и скриптов. Видно только то, что реально смонтировано как файловая система.
- На что смотреть:
- Used/Free
- только файловые системы
- сравнение с Get-Volume
- Что проверить потом:
- Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size
- Get-Disk | Select-Object Number,FriendlyName,HealthStatus,OperationalStatus,Size

#### Физические диски и health

```powershell
Get-Disk | Select-Object Number,FriendlyName,PartitionStyle,HealthStatus,OperationalStatus,Size
```

- Связанная оснастка: Win+X -> Управление дисками
- Когда использовать: Когда нужно понять, проблема в логическом томе или уже в самом диске.
- Назначение: Показывает физические диски, стиль разметки и базовый health-status.
- Осторожно: Если HealthStatus не в норме, не лечи это только очисткой: это уже вопрос состояния железа и storage-пути.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Number FriendlyName       PartitionStyle HealthStatus OperationalStatus         Size
------ ------------       -------------- ------------ -----------------         ----
0      NVMe SSD 1TB       GPT            Healthy      Online            1000202273280
1      USB External Disk  MBR            Healthy      Online             500107862016
```

- Как читать результат: Команда полезна, чтобы не путать проблему файловой системы с проблемой самого устройства.
- На что смотреть:
- HealthStatus
- OperationalStatus
- какой диск системный
- Что проверить потом:
- Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size

#### Содержимое TEMP

```powershell
Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime
```

- Связанная оснастка: Параметры -> Система -> Память
- Когда использовать: Когда нужно понять, что именно копится во временных каталогах перед очисткой.
- Назначение: Показывает, что лежит в пользовательском TEMP и какие файлы там самые заметные.
- Осторожно: Не удаляй временные файлы наугад, если не понимаешь, кто их использует прямо сейчас.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Name                         Length LastWriteTime
----                         ------ -------------
7zO12A.tmp                    84512 19.04.2026 00:54:12
ChromeProfileDump             4096  18.04.2026 22:11:44
Code Cache                    0     18.04.2026 21:07:12
```

- Как читать результат: Смотри на размер, возраст и повторяющиеся шаблоны. Это помогает понять, кто загрязняет TEMP.
- На что смотреть:
- старые файлы
- крупные объекты
- повторяющиеся кэши
- Что проверить потом:
- Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size

### [Windows] Процессы и производительность

#### Тяжёлые процессы по CPU

```powershell
Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS
```

- Связанная оснастка: Win+X -> Диспетчер задач
- Когда использовать: Когда система ощущается тяжелой и нужно быстро найти главного потребителя CPU.
- Назначение: Помогает быстро найти процессы, которые съели больше всего процессорного времени.
- Осторожно: Высокий CPU не всегда означает проблему: индексация, обновления и антивирус могут давать нормальный фон.
- Можно запускать внутри приложения: да

Пример вывода:

```text
ProcessName        CPU      WS
-----------        ---      --
chrome             843.72   952320000
MsMpEng            312.61   284196864
Code               221.04   612794368
OneDrive            74.38   118550528
```

- Как читать результат: CPU показывает накопленное время. Ищи не просто большие числа, а процессы, которые долго растут без понятной причины.
- На что смотреть:
- долгоживущие процессы
- сочетание CPU и WS
- неожиданные имена
- Что проверить потом:
- Get-Counter '\Memory\Available MBytes'
- Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location

#### Свободная память

```powershell
Get-Counter '\Memory\Available MBytes'
```

- Связанная оснастка: Win+X -> Диспетчер задач
- Когда использовать: Когда подозреваешь нехватку памяти, а не процессора.
- Назначение: Показывает, сколько памяти сейчас доступно системе.
- Осторожно: Одна метрика не заменяет полный memory analysis, но дает быстрый sanity-check.
- Можно запускать внутри приложения: да

Пример вывода:

```text
(PDH-CSV 4.0)","\\DESKTOP-01\memory\available mbytes"
"19.04.2026 01:20:12.312","914"
```

- Как читать результат: Если Available MBytes очень низок и держится таким долго, ищи приложение или сервис, который давит на память.
- На что смотреть:
- длительно низкий available memory
- сопоставление с тяжелыми процессами
- Что проверить потом:
- Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS

#### Автозагрузка

```powershell
Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location
```

- Связанная оснастка: Win+X -> Диспетчер задач
- Когда использовать: Когда логон медленный или хочется разобрать, что система запускает без спроса.
- Назначение: Показывает программы, стартующие вместе с Windows.
- Осторожно: Не путай полезные агенты синхронизации и защиты с реально лишним автозапуском.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name                Command                                              Location
----                -------                                              --------
OneDrive            "C:\Users\student\AppData\Local\Microsoft\..."        HKCU\Software\Microsoft\...
SecurityHealth      C:\Windows\system32\SecurityHealthSystray.exe         HKLM\Software\Microsoft\...
Teams Machine       C:\Program Files\Teams Installer\Teams.exe            Startup
```

- Как читать результат: Смотри на путь запуска. AppData, Temp и скрытые пользовательские папки требуют отдельной проверки.
- На что смотреть:
- Location
- пути в AppData
- подписанные и штатные компоненты
- Что проверить потом:
- Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Run','HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'

#### Отчёт по батарее

```powershell
powercfg /batteryreport
```

- Связанная оснастка: Win+X -> Управление электропитанием
- Когда использовать: Когда нужно понять, это тормоза из-за degraded battery/power profile или нет.
- Назначение: Создает HTML-отчет по батарее ноутбука: емкость, циклы, историю использования.
- Осторожно: Команда создает файл-отчет. Практический смысл есть на ноутбуках и планшетах, а не на обычных стационарных ПК.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Battery life report saved to file path
C:\Users\student\battery-report.html
```

- Как читать результат: Полезный результат находится не в консоли, а в HTML-файле. Смотри Design Capacity и Full Charge Capacity.
- На что смотреть:
- путь к HTML-отчету
- Design Capacity
- Full Charge Capacity
- Что проверить потом:
- powercfg /energy

### [Windows] Службы и автозапуск

#### Активные службы

```powershell
Get-Service | Where-Object Status -eq 'Running' | Select-Object -First 20 Name,DisplayName,Status
```

- Связанная оснастка: Win+X -> Управление компьютером
- Когда использовать: Когда нужно понять, что именно сейчас живет на хосте как сервис.
- Назначение: Показывает работающие службы и помогает быстро понять профиль хоста.
- Осторожно: Не отключай службу вслепую: сначала пойми, системная она, защитная или прикладная.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name           DisplayName                               Status
----           -----------                               ------
LanmanServer   Server                                    Running
WinDefend      Microsoft Defender Antivirus Service      Running
wuauserv       Windows Update                            Running
```

- Как читать результат: Смотри на сетевые, защитные и удаленные сервисы. Неожиданное имя без понятной роли - уже повод копать дальше.
- На что смотреть:
- защитные сервисы
- сетевые службы
- службы удаленного доступа
- Что проверить потом:
- Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess

#### Активные задачи планировщика

```powershell
Get-ScheduledTask | Where-Object State -ne 'Disabled' | Select-Object -First 20 TaskName,TaskPath,State
```

- Связанная оснастка: Win+X -> Управление компьютером / taskschd.msc
- Когда использовать: Когда автозапуск неочевиден и нужно проверить не только Run keys.
- Назначение: Показывает включенные scheduled tasks и помогает искать подозрительный автозапуск.
- Осторожно: Перед отключением задачи пойми, не отвечает ли она за драйвер, резервную копию или обновления.
- Можно запускать внутри приложения: да

Пример вывода:

```text
TaskName                         TaskPath                 State
--------                         --------                 -----
GoogleUpdaterTaskMachineCore     \                        Ready
MicrosoftEdgeUpdateTaskMachineUA \                        Ready
OneDrive Standalone Update Task  \                        Running
```

- Как читать результат: Особенно внимательно смотри на задачи из AppData, Temp и user-writable каталогов.
- На что смотреть:
- TaskPath
- Running/Ready
- пути к действиям задачи
- Что проверить потом:
- Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location

#### Run keys

```powershell
Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Run','HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
```

- Связанная оснастка: Win+X -> Терминал (Администратор)
- Когда использовать: Когда нужно добить ревизию автозагрузки после просмотра Task Scheduler и StartupCommand.
- Назначение: Показывает классические точки автозапуска из реестра.
- Осторожно: Особенно внимательно смотри пути из AppData, Temp, Downloads и нестандартных пользовательских каталогов.
- Можно запускать внутри приложения: да

Пример вывода:

```text
SecurityHealth   : C:\Windows\system32\SecurityHealthSystray.exe
OneDrive         : C:\Users\student\AppData\Local\Microsoft\OneDrive\OneDrive.exe
Updater          : C:\Users\student\AppData\Roaming\Vendor\updater.exe
```

- Как читать результат: Оцени не только имя записи, но и фактический путь к файлу. User-writable путь в автозапуске почти всегда требует разбора.
- На что смотреть:
- пути в AppData и Temp
- нештатные названия
- HKCU против HKLM
- Что проверить потом:
- Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location

### [Windows] Журналы и защита

#### Состояние Defender

```powershell
Get-MpComputerStatus
```

- Связанная оснастка: Безопасность Windows / Терминал (Администратор)
- Когда использовать: Когда нужно быстро понять, жива ли базовая защита на хосте.
- Назначение: Показывает состояние Microsoft Defender, сигнатур и real-time protection.
- Осторожно: Если Defender выключен, сначала проверь, не стоит ли другой endpoint protection. Не делай выводы по одному флагу.
- Можно запускать внутри приложения: да

Пример вывода:

```text
AMServiceEnabled          : True
AntivirusEnabled          : True
BehaviorMonitorEnabled    : True
RealTimeProtectionEnabled : True
QuickScanAge              : 1
```

- Как читать результат: Тебя интересуют флаги Enabled, свежесть сигнатур и отключенные защитные компоненты.
- На что смотреть:
- RealTimeProtectionEnabled
- AntivirusEnabled
- возраст сигнатур
- Что проверить потом:
- Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction

#### Профили firewall

```powershell
Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction
```

- Связанная оснастка: Win+X -> Терминал (Администратор)
- Когда использовать: Когда проверяешь exposed services и общую сетевую гигиену хоста.
- Назначение: Показывает, включены ли профили Domain, Private и Public и как они обрабатывают трафик.
- Осторожно: Отключенный Public profile или слишком разрешающий inbound-профиль требует отдельного объяснения.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Name    Enabled DefaultInboundAction DefaultOutboundAction
----    ------- -------------------- ---------------------
Domain  True    NotConfigured        NotConfigured
Private True    Block                Allow
Public  True    Block                Allow
```

- Как читать результат: Для рабочей станции типичный baseline: Public и Private включены, входящие подключения по умолчанию блокируются.
- На что смотреть:
- Enabled
- DefaultInboundAction
- Public profile
- Что проверить потом:
- Get-NetTCPConnection -State Listen | Sort-Object LocalPort | Select-Object -First 20 LocalAddress,LocalPort,OwningProcess

#### Последние системные события

```powershell
Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message
```

- Связанная оснастка: Win+X -> Просмотр событий
- Когда использовать: Когда машина ругается, но еще неясно, смотреть в сеть, службы или storage.
- Назначение: Быстро показывает свежие события системного журнала Windows.
- Осторожно: Системный журнал шумный. Ищи повторяющегося провайдера и повторяющиеся ошибки, а не случайный единичный event.
- Можно запускать внутри приложения: да

Пример вывода:

```text
TimeCreated          Id ProviderName                   LevelDisplayName Message
-----------          -- ------------                   ---------------- -------
19.04.2026 01:05:22 7036 Service Control Manager       Information      The Windows Update service entered the running state.
19.04.2026 01:04:11 10016 DistributedCOM               Warning          The application-specific permission settings do not grant Local Activation...
19.04.2026 01:03:01 6005 EventLog                      Information      The Event log service was started.
```

- Как читать результат: Это команда для поиска паттерна: один и тот же provider, один и тот же ID, одна и та же история ошибок.
- На что смотреть:
- повторяющийся ProviderName
- LevelDisplayName=Error/Warning
- сообщения рядом по времени
- Что проверить потом:
- Get-Service | Where-Object Status -eq 'Running' | Select-Object -First 20 Name,DisplayName,Status

#### Свежие обновления

```powershell
Get-HotFix | Sort-Object InstalledOn -Descending | Select-Object -First 10 HotFixID,InstalledOn,Description
```

- Связанная оснастка: Параметры -> Центр обновления Windows
- Когда использовать: Когда хочешь понять, насколько актуальна система по патчам.
- Назначение: Показывает последние установленные hotfix и обновления.
- Осторожно: Это не полная картина Windows Update, но для первого прохода очень полезно.
- Можно запускать внутри приложения: да

Пример вывода:

```text
HotFixID   InstalledOn Description
--------   ----------- -----------
KB5036893  18.04.2026  Update
KB5035702  18.04.2026  Security Update
KB5035155  10.04.2026  Update
```

- Как читать результат: Полезно видеть, обновлялась ли машина недавно и не застряла ли она на древнем уровне патчей.
- На что смотреть:
- InstalledOn
- security updates
- давно ли не обновлялась машина
- Что проверить потом:
- Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message

### [Linux] Доступ и учётные записи

#### Текущий пользователь и группы

```bash
id
```

- Связанная оснастка: id / groups / sudo
- Когда использовать: Когда нужно понять, под кем ты реально работаешь и какие группы расширяют твои возможности.
- Назначение: Показывает текущего пользователя, uid, gid и группы.
- Осторожно: Это базовая команда чтения. На сервере она помогает сразу понять, работаешь ли ты под нужной учеткой.
- Можно запускать внутри приложения: да

Пример вывода:

```text
uid=1000(student) gid=1000(student) groups=1000(student),4(adm),27(sudo),999(docker)
```

- Как читать результат: Лишняя группа sudo, wheel, docker или adm уже меняет картину риска и видимости системы.
- На что смотреть:
- наличие sudo/wheel/docker
- uid/gid
- лишние группы
- Что проверить потом:
- sudo -l
- last -a | head -n 10

#### Локальные учётные записи

```bash
getent passwd | head -n 10
```

- Связанная оснастка: getent / /etc/passwd
- Когда использовать: Когда нужно понять, какие локальные пользователи есть и кто из них интерактивный.
- Назначение: Показывает первые локальные записи из passwd и напоминает, как выглядит структура пользователей.
- Осторожно: На production-хостах полный список обычно длиннее. Здесь полезен сам формат и первичная инвентаризация.
- Можно запускать внутри приложения: да

Пример вывода:

```text
root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
student:x:1000:1000:Student:/home/student:/bin/bash
```

- Как читать результат: Смотри на shell, uid и домашний каталог. `/usr/sbin/nologin` и `/bin/false` отделяют сервисные учетные записи от интерактивных.
- На что смотреть:
- shell пользователя
- uid<1000 против uid>=1000
- домашний каталог
- Что проверить потом:
- id
- last -a | head -n 10

#### Права sudo

```bash
sudo -l
```

- Связанная оснастка: sudoers / /etc/sudoers.d
- Когда использовать: Когда нужно понять, ограничены ли права пользователя или он фактически администратор.
- Назначение: Показывает, какие команды текущий пользователь может запускать через sudo.
- Осторожно: Команда часто требует пароль и дает чувствительную информацию о правах. Используй ее только на своих системах и в рамках аудита.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Matching Defaults entries for student on lab:
    env_reset, mail_badpass, secure_path=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

User student may run the following commands on lab:
    (ALL : ALL) ALL
```

- Как читать результат: Если видишь ALL или NOPASSWD на широкий набор команд, это очень сильные привилегии.
- На что смотреть:
- NOPASSWD
- ALL : ALL
- узко ограниченные команды
- Что проверить потом:
- id
- last -a | head -n 10

#### История входов

```bash
last -a | head -n 10
```

- Связанная оснастка: last / journalctl -u ssh
- Когда использовать: Когда нужно быстро посмотреть, были ли интерактивные входы и откуда.
- Назначение: Показывает последние интерактивные входы и источники подключений.
- Осторожно: На контейнерах и свежих лабораториях вывод может быть пустым. Это не ошибка, а особенность среды.
- Можно запускать внутри приложения: да

Пример вывода:

```text
student  pts/0        192.168.1.20     Fri Apr 19 00:51   still logged in
reboot   system boot  6.6.36.6-microsoft Fri Apr 19 00:48
```

- Как читать результат: Ищи неожиданные внешние IP и непривычные окна времени. Потом связывай это с SSH-логами.
- На что смотреть:
- IP-адрес источника
- время входа
- still logged in
- Что проверить потом:
- sudo journalctl -u ssh -n 20
- id

### [Linux] Сеть и DNS

#### Сетевые интерфейсы

```bash
ip addr show
```

- Связанная оснастка: ip / NetworkManager / systemd-networkd
- Когда использовать: Когда сервис недоступен и сначала нужно понять, жив ли вообще сетевой стек хоста.
- Назначение: Показывает сетевые интерфейсы Linux и назначенные адреса.
- Осторожно: Это современная замена ifconfig на большинстве дистрибутивов.
- Можно запускать внутри приложения: да

Пример вывода:

```text
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500
    inet 192.168.1.52/24 brd 192.168.1.255 scope global eth0
3: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500
    inet 172.17.0.1/16 scope global docker0
```

- Как читать результат: Отделяй основной интерфейс от виртуальных мостов Docker, VPN и hypervisor-сетей.
- На что смотреть:
- основной интерфейс
- IPv4/IPv6
- виртуальные интерфейсы
- Что проверить потом:
- ip route
- ss -tulpn

#### Маршруты

```bash
ip route
```

- Связанная оснастка: ip / routing
- Когда использовать: Когда хост не видит внешние адреса или странно идет только локальный трафик.
- Назначение: Показывает таблицу маршрутизации Linux.
- Осторожно: Без маршрута по умолчанию выход наружу невозможен, как бы хорошо ни выглядел интерфейс.
- Можно запускать внутри приложения: да

Пример вывода:

```text
default via 192.168.1.1 dev eth0 proto dhcp src 192.168.1.52 metric 100
172.17.0.0/16 dev docker0 proto kernel scope link src 172.17.0.1
```

- Как читать результат: Если нет default route, проблема уже на уровне сетевой конфигурации или DHCP, а не у приложения.
- На что смотреть:
- default via
- какой dev используется
- виртуальные подсети
- Что проверить потом:
- ip addr show
- getent hosts example.org

#### Проверка DNS

```bash
getent hosts example.org
```

- Связанная оснастка: getent / resolv.conf
- Когда использовать: Когда нужно отделить проблему резолвинга от проблемы соединения.
- Назначение: Проверяет, как система резолвит имя через настроенные механизмы NSS/DNS.
- Осторожно: Это удобнее для базовой проверки, чем сразу лезть в dig, особенно на минимальных системах.
- Можно запускать внутри приложения: да

Пример вывода:

```text
93.184.216.34   example.org
```

- Как читать результат: Если имя не резолвится, дальше нет смысла проверять TCP-порт до исправления DNS.
- На что смотреть:
- есть ли IP в ответе
- неожиданный адрес
- вообще есть ли ответ
- Что проверить потом:
- ss -tulpn
- ip route

#### Слушающие порты

```bash
ss -tulpn
```

- Связанная оснастка: ss / systemctl / journalctl
- Когда использовать: Когда нужно быстро сопоставить сервис, порт и процесс.
- Назначение: Показывает TCP и UDP listeners вместе с owning process.
- Осторожно: На production-хостах лучше фильтровать вывод по конкретному порту, чтобы не тонуть в шуме.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Netid State  Local Address:Port Process
tcp   LISTEN 0.0.0.0:22       users:(("sshd",pid=728,fd=3))
tcp   LISTEN 127.0.0.1:5432   users:(("postgres",pid=991,fd=5))
tcp   LISTEN 0.0.0.0:8080     users:(("java",pid=2044,fd=84))
```

- Как читать результат: Принцип тот же, что и на Windows: кто слушает, на каком адресе и должен ли этот сервис торчать наружу.
- На что смотреть:
- 0.0.0.0 против 127.0.0.1
- неожиданные порты
- имя процесса
- Что проверить потом:
- systemctl --type=service --state=running --no-pager | head -n 20

### [Linux] Диски и файловые системы

#### Свободное место на файловых системах

```bash
df -h
```

- Связанная оснастка: df / storage
- Когда использовать: Когда нужно быстро понять, какой mount point на грани.
- Назначение: Показывает заполнение файловых систем в удобочитаемом формате.
- Осторожно: Не путай обычные тома с tmpfs и контейнерными mounts.
- Можно запускать внутри приложения: да

Пример вывода:

```text
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda2        80G   62G   14G  82% /
tmpfs           3.8G  1.2M  3.8G   1% /run
/dev/sdb1       200G   91G  100G  48% /data
```

- Как читать результат: Если системный раздел перевалил за 80-90 процентов, пора смотреть логи, кэши и контейнерные данные.
- На что смотреть:
- Use%
- корневой раздел
- tmpfs отдельно от обычных дисков
- Что проверить потом:
- lsblk -f
- findmnt

#### Блочные устройства

```bash
lsblk -f
```

- Связанная оснастка: lsblk / findmnt
- Когда использовать: Когда нужно понять топологию дисков и разделов, а не только проценты заполнения.
- Назначение: Показывает блочные устройства, файловые системы, UUID и точки монтирования.
- Осторожно: Это карта хранилища. Она нужна, чтобы не перепутать физический диск, раздел и точку монтирования.
- Можно запускать внутри приложения: да

Пример вывода:

```text
NAME   FSTYPE LABEL UUID                                 MOUNTPOINT
sda
├─sda1 ext4         2b99b1d8-2a1a-4f9c-8bd3-a9d1d1c2a900 /boot
└─sda2 ext4 root    d551a8db-4518-4f4d-b91c-87f5a0af1100 /
sdb1 xfs  data      8f84b0b9-00e9-430d-8d5d-30dd1aef96b1 /data
```

- Как читать результат: Команда помогает быстро увидеть, где root, где data и что реально смонтировано.
- На что смотреть:
- FSTYPE
- MOUNTPOINT
- какое устройство системное
- Что проверить потом:
- df -h
- findmnt

#### Точки монтирования

```bash
findmnt
```

- Связанная оснастка: findmnt / mount
- Когда использовать: Когда путаница именно в mount points, а не в общем объеме диска.
- Назначение: Показывает дерево точек монтирования и реальные source/target пары.
- Осторожно: Очень полезно на системах с контейнерами, overlayfs и большим числом mount points.
- Можно запускать внутри приложения: да

Пример вывода:

```text
TARGET SOURCE           FSTYPE OPTIONS
/      /dev/sda2        ext4   rw,relatime
/boot  /dev/sda1        ext4   rw,relatime
/data  /dev/sdb1        xfs    rw,relatime
```

- Как читать результат: Команда хороша, когда нужно не просто знать размер, а понимать, откуда и куда смонтирован каждый том.
- На что смотреть:
- TARGET
- SOURCE
- overlay/tmpfs
- Что проверить потом:
- df -h
- lsblk -f

### [Linux] Процессы и сервисы

#### Тяжёлые процессы по памяти

```bash
ps aux --sort=-%mem | head -n 10
```

- Связанная оснастка: ps / top / htop
- Когда использовать: Когда подозреваешь memory pressure и нужно найти главного потребителя.
- Назначение: Показывает процессы, которые занимают больше всего памяти.
- Осторожно: Для live-наблюдения удобнее top/htop, но этот срез хорош для отчетов и заметок.
- Можно запускать внутри приложения: да

Пример вывода:

```text
USER   PID %CPU %MEM COMMAND
mysql  944  5.1 28.4 /usr/sbin/mysqld
java  2011  9.7 14.2 /usr/bin/java -jar app.jar
root   728  0.0  0.6 sshd: /usr/sbin/sshd -D
```

- Как читать результат: Смотри не только на %MEM, но и на роль процесса. База данных может есть память штатно, а случайный java/node-процесс - уже повод разбираться.
- На что смотреть:
- %MEM
- роль процесса
- неожиданные демоны
- Что проверить потом:
- ps aux --sort=-%cpu | head -n 10
- systemctl --type=service --state=running --no-pager | head -n 20

#### Тяжёлые процессы по CPU

```bash
ps aux --sort=-%cpu | head -n 10
```

- Связанная оснастка: ps / top / htop
- Когда использовать: Когда хост тормозит и нужно быстро локализовать потребителя CPU.
- Назначение: Показывает процессы, которые потребляют больше всего CPU.
- Осторожно: Один короткий всплеск не всегда проблема. Ищи устойчивую нагрузку и повторяемый pattern.
- Можно запускать внутри приложения: да

Пример вывода:

```text
USER   PID %CPU %MEM COMMAND
java  2011 79.3 14.2 /usr/bin/java -jar app.jar
root  1448 12.8  1.1 /usr/bin/containerd
mysql  944  5.1 28.4 /usr/sbin/mysqld
```

- Как читать результат: CPU сам по себе мало о чем говорит, если не понимаешь роль процесса и время наблюдения.
- На что смотреть:
- %CPU
- повторяющиеся процессы
- службы без понятной роли
- Что проверить потом:
- systemctl --type=service --state=running --no-pager | head -n 20
- ss -tulpn

#### Активные systemd-сервисы

```bash
systemctl --type=service --state=running --no-pager | head -n 20
```

- Связанная оснастка: systemctl / journalctl
- Когда использовать: Когда нужно понять, какие демоны реально запущены, а не просто установлены.
- Назначение: Показывает работающие systemd-сервисы и помогает понять профиль машины.
- Осторожно: На минимальных образах или контейнерах systemd может быть не основным init, и команда будет менее полезна.
- Можно запускать внутри приложения: да

Пример вывода:

```text
ssh.service          loaded active running OpenBSD Secure Shell server
cron.service         loaded active running Regular background program processing daemon
docker.service       loaded active running Docker Application Container Engine
```

- Как читать результат: По списку можно быстро понять роль машины: jump-host, application server, container node и так далее.
- На что смотреть:
- unexpected services
- роль хоста
- сетевые/удаленные сервисы
- Что проверить потом:
- ss -tulpn
- ps aux --sort=-%mem | head -n 10

#### Нагрузка системы

```bash
uptime
```

- Связанная оснастка: uptime / top
- Когда использовать: Когда нужен очень быстрый health-check по нагрузке хоста.
- Назначение: Показывает время работы системы и load average.
- Осторожно: Load average нужно читать вместе с числом CPU и типом нагрузки, а не как абстрактную страшную цифру.
- Можно запускать внутри приложения: да

Пример вывода:

```text
 01:42:18 up 2 days,  4:22,  2 users,  load average: 0.42, 0.51, 0.56
```

- Как читать результат: Стабильно высокий load без понятной причины - повод перейти к top/ps и смотреть, кто грузит систему.
- На что смотреть:
- load average 1/5/15
- время работы
- число пользователей
- Что проверить потом:
- ps aux --sort=-%cpu | head -n 10

### [Linux] Журналы и безопасность

#### Критичные события журнала

```bash
journalctl -p 3 -xb | tail -n 20
```

- Связанная оснастка: journalctl
- Когда использовать: Когда нужно быстро понять, о чем система чаще всего жалуется прямо сейчас.
- Назначение: Показывает ошибки текущей загрузки системы из journalctl.
- Осторожно: На некоторых системах нужен sudo или доступ к группе systemd-journal.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Apr 19 00:58:10 lab kernel: EXT4-fs warning (device sda2): ext4_end_bio:344: I/O error 10 writing to inode 524312
Apr 19 00:58:12 lab sshd[742]: error: maximum authentication attempts exceeded for invalid user admin
```

- Как читать результат: Ищи не единичную строку, а повторяющийся тип ошибки: storage, auth, network, service startup.
- На что смотреть:
- повторяющиеся ошибки
- storage/auth/network
- текущая загрузка
- Что проверить потом:
- systemctl --type=service --state=running --no-pager | head -n 20

#### SSH-журнал

```bash
sudo journalctl -u ssh -n 20
```

- Связанная оснастка: journalctl / auth.log
- Когда использовать: Когда анализируешь удаленный доступ и подозреваешь перебор или несанкционированный вход.
- Назначение: Показывает последние события SSH: логины, ошибки и brute-force попытки.
- Осторожно: Обычно нужны повышенные права. Не запускай бездумно на чужих системах.
- Можно запускать внутри приложения: нет

Пример вывода:

```text
Apr 19 00:51:02 lab sshd[742]: Accepted password for student from 192.168.1.20 port 50122 ssh2
Apr 19 00:53:11 lab sshd[756]: Failed password for invalid user admin from 185.22.44.9 port 45118 ssh2
```

- Как читать результат: Успешные и неуспешные входы нужно читать вместе: кто стучался, откуда и был ли потом успешный логин тем же источником.
- На что смотреть:
- Accepted/Failed password
- IP источника
- повторяющийся brute-force
- Что проверить потом:
- last -a | head -n 10
- id

## Системные инструменты и оснастки

### [Windows] Win+X и системные оснастки

#### Установленные приложения

- Где открыть: `Win+X -> Установленные приложения | ms-settings:appsfeatures`
- Для чего нужен: Показывает список установленного ПО, издателя, версию и помогает найти лишние программы, старые агенты и автообновляторы.
- Когда открывать:
- Когда нужно понять, откуда взялось незнакомое приложение или служба.
- Когда система захламлена и нужно убрать лишний софт перед оптимизацией.
- Когда проверяешь, есть ли на машине антивирус, VPN-клиент, драйверный пакет или корпоративный агент.
- Что смотреть:
- Недавно установленные программы и неизвестные издатели.
- Тяжелые программы, которые ставят автозапуск, фоновые службы или телеметрию.
- Дублирующее ПО: несколько архиваторов, браузеров, VPN-клиентов, Java/.NET runtimes.
- Какие команды помогают:
- winget list
- Get-Package
- Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*' | Select-Object DisplayName,Publisher,DisplayVersion

#### Управление электропитанием

- Где открыть: `Win+X -> Управление электропитанием | powercfg.cpl`
- Для чего нужен: Нужно для профилей питания, сна, гибернации и общей оптимизации ноутбука или рабочей станции.
- Когда открывать:
- Когда ноутбук быстро разряжается или уходит в сон слишком агрессивно.
- Когда ПК работает медленно из-за экономичного профиля питания.
- Когда нужно понять, почему устройство не засыпает или просыпается само.
- Что смотреть:
- Активный план питания и его поведение на батарее и от сети.
- Параметры сна, гибернации, действий при закрытии крышки и кнопки питания.
- Устройства и процессы, которые будят систему или мешают сну.
- Какие команды помогают:
- powercfg /list
- powercfg /query
- powercfg /batteryreport
- powercfg /requests

#### Просмотр событий

- Где открыть: `Win+X -> Просмотр событий | eventvwr.msc`
- Для чего нужен: Главная графическая точка входа в журналы Windows: ошибки служб, драйверов, входов, обновлений и системных сбоев.
- Когда открывать:
- Когда после обновления или установки драйвера что-то перестало работать.
- Когда нужно понять, что происходило перед падением системы или приложения.
- Когда расследуешь входы, ошибки служб, блокировки и сообщения безопасности.
- Что смотреть:
- Windows Logs -> System для служб, драйверов, сетевых и дисковых ошибок.
- Windows Logs -> Security для входов, блокировок и аудита доступа.
- Applications and Services Logs для Defender, PowerShell и других подсистем.
- Какие команды помогают:
- Get-WinEvent -LogName System -MaxEvents 20
- Get-WinEvent -LogName Security -MaxEvents 20
- Get-WinEvent -ListLog * | Sort-Object RecordCount -Descending | Select-Object -First 20

#### Система

- Где открыть: `Win+X -> Система | ms-settings:about`
- Для чего нужен: Даёт быстрый обзор устройства: имя хоста, редакция Windows, версия, объём памяти и базовые сведения о системе.
- Когда открывать:
- Когда нужно быстро собрать профиль компьютера для отчёта, заявки или аудита.
- Когда сравниваешь требования ПО с реальной конфигурацией машины.
- Когда проверяешь, какая у хоста редакция Windows и какой build установлен.
- Что смотреть:
- Имя компьютера и текущее доменное/рабочее окружение.
- Версию Windows, build и объём оперативной памяти.
- Связанные ссылки на расширенные свойства системы и защитные настройки.
- Какие команды помогают:
- Get-ComputerInfo | Select-Object CsName,WindowsProductName,WindowsVersion,OsBuildNumber,OsHardwareAbstractionLayer
- systeminfo

#### Диспетчер устройств

- Где открыть: `Win+X -> Диспетчер устройств | devmgmt.msc`
- Для чего нужен: Нужен для драйверов, устройств с ошибками, отключённых адаптеров и проблем с оборудованием.
- Когда открывать:
- Когда не работает сеть, звук, Bluetooth, камера или внешний накопитель.
- Когда после обновления системы появились ошибки драйверов.
- Когда нужно проверить, не отключено ли устройство политикой или вручную.
- Что смотреть:
- Жёлтые предупреждения, неизвестные устройства и коды ошибок.
- Состояние сетевых адаптеров, накопителей, видеокарты и USB-контроллеров.
- Версию драйвера и дату выпуска для критичных устройств.
- Какие команды помогают:
- Get-PnpDevice | Where-Object Status -ne 'OK'
- driverquery
- pnputil /enum-drivers

#### Сетевые подключения

- Где открыть: `Win+X -> Сетевые подключения | ncpa.cpl`
- Для чего нужен: Графическая точка для просмотра адаптеров, их статуса, IPv4/IPv6, DNS и отключения лишних интерфейсов.
- Когда открывать:
- Когда нет интернета, не работает VPN или пропадает доступ к доменным ресурсам.
- Когда на машине несколько адаптеров и нужно понять, какой реально используется.
- Когда нужно быстро проверить, не отключён ли адаптер и не прописан ли вручную странный DNS.
- Что смотреть:
- Состояние адаптера: включён, отключён, сетевой кабель не подключён.
- Ручные IP/DNS-настройки, мосты, виртуальные адаптеры VPN или гипервизора.
- Лишние интерфейсы, которые путают маршрутизацию или ломают приоритет сетей.
- Какие команды помогают:
- Get-NetAdapter
- Get-NetIPConfiguration
- Get-DnsClientServerAddress
- Test-NetConnection example.org -Port 443

#### Управление дисками

- Где открыть: `Win+X -> Управление дисками | diskmgmt.msc`
- Для чего нужен: Нужно для разметки дисков, букв томов, состояния разделов и диагностики storage-проблем.
- Когда открывать:
- Когда диск не виден в Проводнике или пропала буква тома.
- Когда нужно понять, откуда взялось мало места или почему не монтируется том.
- Когда подключён новый диск, SSD или VHD и нужно проверить его состояние.
- Что смотреть:
- Онлайн ли диск, есть ли буква тома, не осталось ли неразмеченное пространство.
- Файловую систему, размер разделов и состояние томов.
- Системные, скрытые и recovery-разделы, которые не стоит трогать без понимания роли.
- Какие команды помогают:
- Get-Disk
- Get-Partition
- Get-Volume

#### Управление компьютером

- Где открыть: `Win+X -> Управление компьютером | compmgmt.msc`
- Для чего нужен: Сводная MMC-консоль для локальных пользователей, общих папок, журналов, планировщика и других оснасток.
- Когда открывать:
- Когда нужен обзор сразу по нескольким админским разделам.
- Когда хочешь показать преподавателю или пользователю единый путь до разных оснасток.
- Когда удобнее идти через MMC, а не помнить все отдельные .msc-файлы.
- Что смотреть:
- Local Users and Groups, Shared Folders, Device Manager, Disk Management.
- Event Viewer и другие оснастки, полезные для диагностики и аудита.
- Какие ветки доступны только на Pro/Enterprise и при повышенных правах.
- Какие команды помогают:
- Get-LocalUser
- Get-SmbShare
- Get-Service
- Get-ScheduledTask

#### Терминал

- Где открыть: `Win+X -> Терминал | wt.exe`
- Для чего нужен: Основная рабочая точка для PowerShell, CMD и автоматизации без повышения привилегий.
- Когда открывать:
- Когда достаточно чтения логов, сбора инвентаря и диагностики без admin-rights.
- Когда изучаешь команды и хочешь безопасно повторять read-only сценарии.
- Когда нужен быстрый запуск PowerShell-утилит, скриптов и конвейеров.
- Что смотреть:
- Текущий пользователь и контекст сеанса.
- Ошибки прав доступа: они подсказывают, где реально нужен elevated shell.
- Повторяемые команды, которые лучше оформить в скрипт или заметку.
- Какие команды помогают:
- Get-Help Get-Service
- Get-Command *Net*
- Get-Location

#### Терминал (Администратор)

- Где открыть: `Win+X -> Терминал (Администратор) | elevated wt.exe`
- Для чего нужен: Нужен только для команд, которые реально требуют повышенных прав: часть журналов, firewall, драйверы, службы и системные настройки.
- Когда открывать:
- Когда обычный терминал упирается в Access Denied.
- Когда нужно читать или менять защищённые системные настройки.
- Когда проверяешь сервисы, драйверы, firewall и некоторые журналы безопасности.
- Что смотреть:
- Действительно ли задача требует admin-rights или это просто привычка.
- Какие команды безопасно читать, а какие уже меняют состояние системы.
- Что запуск от имени администратора повышает риск случайных изменений.
- Какие команды помогают:
- whoami /groups
- Get-NetFirewallProfile
- Get-WinEvent -LogName Security -MaxEvents 20

#### Диспетчер задач

- Где открыть: `Win+X -> Диспетчер задач | taskmgr`
- Для чего нужен: Быстрый визуальный инструмент для процессов, автозагрузки, нагрузки CPU/RAM/диска и базовой оптимизации.
- Когда открывать:
- Когда компьютер тормозит и нужно быстро найти самый тяжёлый процесс.
- Когда нужно оценить, что грузит диск, память или сеть.
- Когда проверяешь автозагрузку и влияние приложений на старт системы.
- Что смотреть:
- Вкладку Processes для текущей нагрузки и трендов по CPU/RAM/Disk/Network.
- Вкладку Startup apps для тяжёлого или лишнего автозапуска.
- Подозрительные процессы с неясным именем, отсутствующим издателем или странным путём.
- Какие команды помогают:
- Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS
- Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location
- Get-Counter '\Processor(_Total)\% Processor Time'

#### Параметры

- Где открыть: `Win+X -> Параметры | ms-settings:`
- Для чего нужен: Современный центр настройки Windows: обновления, приложения, сеть, конфиденциальность, учётки и восстановление.
- Когда открывать:
- Когда пользователю проще показать GUI-путь, чем диктовать команду.
- Когда нужно связать командную диагностику с понятной графической настройкой.
- Когда оформляешь учебный сценарий 'где это найти в интерфейсе'.
- Что смотреть:
- Разделы Windows Update, Apps, Network & Internet, Accounts, Privacy & Security.
- Какая настройка меняется сразу, а какая только перенаправляет в старую панель управления.
- Какие экраны полезны для объяснения новичку: память, автозапуск, обновления, восстановление.
- Какие команды помогают:
- start ms-settings:windowsupdate
- start ms-settings:network-status
- start ms-settings:appsfeatures

#### Выполнить

- Где открыть: `Win+X -> Выполнить | Win+R`
- Для чего нужен: Самый быстрый способ открыть системные .msc, панели и диагностические утилиты по имени.
- Когда открывать:
- Когда нужно быстро попасть в eventvwr.msc, services.msc, taskschd.msc или ncpa.cpl.
- Когда строишь себе короткие маршруты по админским оснасткам.
- Когда хочешь показать пользователю простой повторяемый способ открыть нужную настройку.
- Что смотреть:
- Какие команды открывают современные Settings, а какие классические mmc/cpl-оснастки.
- Какие ярлыки реально ускоряют работу: eventvwr.msc, devmgmt.msc, diskmgmt.msc, ms-settings:*.
- Что Run отлично подходит для обучения навигации по Windows.
- Какие команды помогают:
- eventvwr.msc
- services.msc
- taskschd.msc
- ncpa.cpl
- powercfg.cpl

## Практические сценарии

### [Windows] Сеть

#### Сайт не открывается

- Симптом: Браузер не открывает сайт, но Wi-Fi или Ethernet вроде бы подключены.
- Цель: Отделить проблему DNS, маршрута, локального интерфейса и удаленного порта.
- Что сделать:
- Проверь, есть ли у активного интерфейса корректный IPv4-адрес, шлюз и DNS.
- Отдельно проверь резолвинг имени сайта и доступность TCP-порта 443.
- Если имя резолвится, но порт не открывается, ищи фильтрацию на пути или отказ сервиса.
- Команды:
- Get-NetIPConfiguration
- Resolve-DnsName example.org
- Test-NetConnection example.org -Port 443
- На что смотреть:
- Активный интерфейс не должен висеть без gateway.
- DNS должен вернуть адрес, иначе проблема в резолвинге.
- TcpTestSucceeded=False при рабочем DNS обычно указывает на сеть, firewall или удаленный сервис.
- Куда еще зайти:
- Win+X -> Сетевые подключения
- Параметры -> Сеть и Интернет

#### Найти лишний открытый порт

- Симптом: Нужно понять, какие сервисы слушают входящие подключения на машине.
- Цель: Быстро связать порт с процессом и решить, нужен ли он вообще.
- Что сделать:
- Выведи слушающие TCP-порты и отметь сервисные порты удаленного доступа.
- Если видишь 445, 3389, 5985 или нестандартный порт, выясни owning process.
- Сопоставь процесс с ролью хоста и реши, должен ли этот сервис быть открыт.
- Команды:
- Get-NetTCPConnection -State Listen | Sort-Object LocalPort
- Get-Service | Where-Object Status -eq 'Running'
- На что смотреть:
- Порт на 0.0.0.0 виден всем интерфейсам, а не только локально.
- 127.0.0.1 обычно безопаснее, чем 0.0.0.0.
- Любой неизвестный listener требует привязки к процессу и назначению.
- Куда еще зайти:
- Win+X -> Терминал (Администратор)
- Win+X -> Диспетчер задач

### [Windows] Диски

#### Заканчивается место на системном диске

- Симптом: Windows начал обновляться с ошибками, хранилище почти заполнено или приложения тормозят.
- Цель: Понять, на каком томе проблема и где искать мусор или лишние данные.
- Что сделать:
- Проверь свободное место по всем томам и отдельно системный диск.
- Посмотри временные каталоги и стандартные точки накопления мусора.
- Перед очисткой убедись, что речь не о recovery/system разделах.
- Команды:
- Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size
- Get-PSDrive -PSProvider FileSystem
- Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime
- На что смотреть:
- Если на системном томе осталось меньше 10-15 процентов, жди проблем с обновлениями и кэшем.
- Recovery-раздел не чистят вручную без четкого понимания последствий.
- Большой и старый TEMP обычно говорит, что есть смысл запускать безопасную очистку.
- Куда еще зайти:
- Win+X -> Управление дисками
- Параметры -> Система -> Память

### [Windows] Процессы и производительность

#### Компьютер долго входит в систему

- Симптом: После логина долго открывается рабочий стол, вентиляторы шумят, система ощущается тяжелой.
- Цель: Разделить проблему на автозагрузку, прожорливые процессы и нехватку памяти.
- Что сделать:
- Сначала посмотри процессы с максимальным CPU или памятью.
- Потом проверь автозагрузку и отличи реально нужные агенты от лишних.
- Если памяти мало, проверь метрику свободной памяти и ищи долгоживущий виновный процесс.
- Команды:
- Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS
- Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location
- Get-Counter '\Memory\Available MBytes'
- На что смотреть:
- CPU показывает накопленное время, а WS отражает рабочий набор памяти.
- Автозагрузка из AppData и user-writable путей требует отдельной проверки.
- Очень низкий Available MBytes обычно означает давление на память и активный paging.
- Куда еще зайти:
- Win+X -> Диспетчер задач
- Win+X -> Установленные приложения

### [Windows] Журналы и защита

#### Нужно понять, почему система ругается на безопасность

- Симптом: Есть предупреждения по защите, firewall или системным событиям, но неясно, куда смотреть первым.
- Цель: Собрать защитную картину: Defender, firewall и последние критичные системные события.
- Что сделать:
- Проверь, включены ли защитные профили и real-time protection.
- Посмотри firewall-профили и базовую inbound/outbound политику.
- После этого прочитай последние системные события, а не только Security.
- Команды:
- Get-MpComputerStatus
- Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction
- Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message
- На что смотреть:
- Старые сигнатуры или отключенный real-time protection требуют объяснения.
- Public-профиль firewall не должен быть выключен без причины.
- В системных событиях ищи повторяющиеся ошибки одного provider, а не единичный шум.
- Куда еще зайти:
- Win+X -> Просмотр событий
- Безопасность Windows

### [Linux] Сеть

#### Linux-хост не видит сеть

- Симптом: Сервис в Linux не отвечает, и нужно быстро отделить проблему интерфейса от маршрута.
- Цель: Проверить адреса, маршрут по умолчанию и слушающие порты сервиса.
- Что сделать:
- Сначала посмотри адреса интерфейсов и не перепутай реальный интерфейс с docker0 или bridge.
- Проверь маршрут по умолчанию и имя шлюза.
- Если сервис должен быть доступен, выведи listeners и адреса привязки.
- Команды:
- ip addr show
- ip route
- ss -tulpn
- На что смотреть:
- Отсутствие default route объясняет невозможность выйти наружу.
- 127.0.0.1 и 0.0.0.0 несут разный смысл для доступности сервиса.
- Bridge и container-сети не должны путать основную картину хоста.
- Куда еще зайти:
- NetworkManager / systemd-networkd
- journalctl -u NetworkManager

### [Linux] Диски

#### Заполняется корневой раздел

- Симптом: На Linux начинаются ошибки записи, падают сервисы или обновления из-за нехватки места.
- Цель: Найти перегруженную файловую систему и понять, кто съел место.
- Что сделать:
- Начни с общей картины по файловым системам.
- Потом проверь блочные устройства и точки монтирования.
- Если корень близок к 100 процентам, отдельно смотри /var/log, контейнеры и кэш пакетов.
- Команды:
- df -h
- lsblk -f
- du -sh /var/log/* 2>/dev/null | sort -h | tail -n 10
- На что смотреть:
- tmpfs не нужно лечить как обычный диск.
- Системный раздел за 80-90 процентов уже требует действий.
- Резкий рост в /var/log часто связан с циклической ошибкой сервиса.
- Куда еще зайти:
- lsblk / findmnt
- journalctl

### [Linux] Доступ и журналы

#### Проверить, кто логинился и какие права есть

- Симптом: Нужно быстро разобраться в локальных учетках, sudo и истории входов.
- Цель: Получить минимальную картину по пользователям, sudo и недавним логинам.
- Что сделать:
- Сначала выведи текущего пользователя и его группы.
- Потом проверь sudo-права и историю логинов.
- Если видишь неожиданный логин, переходи к SSH-логам и журналам auth.
- Команды:
- id
- sudo -l
- last -a | head -n 10
- На что смотреть:
- Лишняя группа sudo или wheel - уже повод на ревизию доступа.
- Неожиданный удаленный IP в last нужно сопоставлять с SSH-журналом.
- Пустой вывод last возможен на свежей системе или в контейнере.
- Куда еще зайти:
- journalctl -u ssh
- /var/log/auth.log

### [Linux] Процессы и сервисы

#### Найти тяжёлый процесс

- Симптом: Хост тормозит, но пока неясно, виноват CPU, память или конкретный демон.
- Цель: Отделить нагрузку CPU, память и профиль запущенных сервисов.
- Что сделать:
- Сначала отсортируй процессы по памяти и CPU.
- Потом посмотри список активных сервисов и роль машины.
- Если процесс непонятен, связывай его с unit, портом и владельцем.
- Команды:
- ps aux --sort=-%mem | head -n 10
- ps aux --sort=-%cpu | head -n 10
- systemctl --type=service --state=running --no-pager | head -n 20
- На что смотреть:
- Рост памяти базы данных может быть нормой, а случайный пользовательский процесс - нет.
- Высокий CPU без понятной роли часто быстрее ловится через связку ps + systemctl + ss.
- Сервисный профиль хоста должен совпадать с тем, для чего этот сервер нужен.
- Куда еще зайти:
- systemctl
- top / htop

## Живые результаты команд

### [Linux] Практика

#### WSL не готов

- Статус: INFO/ERROR
- Для чего: Служебная информация о практическом режиме.
- Примечание: Linux live-проверки пропущены: либо `wsl.exe` недоступен, либо в системе еще не установлен Linux-дистрибутив.

```text
(no output)
```

### [Windows] Учётные записи и доступ

#### Локальные пользователи

```powershell
Get-LocalUser
```

- Статус: OK
- Для чего: Показывает локальные учетные записи Windows, их состояние и базовые свойства.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Name                Enabled Description                                                                                
----                ------- -----------                                                                                
CodexSandboxOffline True                                                                                               
CodexSandboxOnline  True                                                                                               
DefaultAccount      False   Учетная запись пользователя, управляемая системой.                                         
smirn               True                                                                                               
WDAGUtilityAccount  False   Учетная запись пользователя, которая управляется и используется системой для сценариев A...
Администратор       False   Встроенная учетная запись администратора компьютера/домена                                 
Гость               False   Встроенная учетная запись для доступа гостей к компьютеру или домену                       



```

#### Парольная политика

```powershell
net accounts
```

- Статус: OK
- Для чего: Показывает локальную парольную политику: длину, срок действия, историю и параметры блокировки.
- Примечание: Команда выполнена через локальный PowerShell.

```text
Force user logoff how long after time expires?:       Never
Minimum password age (days):                          0
Maximum password age (days):                          42
Minimum password length:                              0
Length of password history maintained:                None
Lockout threshold:                                    10
Lockout duration (minutes):                           10
Lockout observation window (minutes):                 10
Computer role:                                        WORKSTATION
The command completed successfully.


```

### [Windows] Сеть и DNS

#### IP-конфигурация

```powershell
Get-NetIPConfiguration
```

- Статус: OK
- Для чего: Показывает активные интерфейсы, IPv4/IPv6, шлюзы и DNS.
- Примечание: Команда выполнена через локальный PowerShell.

```text


InterfaceAlias       : outline-tap0
InterfaceIndex       : 4
InterfaceDescription : TAP-Windows Adapter V9
NetProfile.Name      : Сеть 2
IPv4Address          : 10.0.85.2
IPv6DefaultGateway   : 
IPv4DefaultGateway   : 
DNSServer            : 1.1.1.1
                       9.9.9.9

InterfaceAlias       : Беспроводная сеть
InterfaceIndex       : 11
InterfaceDescription : Qualcomm WCN685x Wi-Fi 6E Dual Band Simultaneous (DBS) WiFiCx Network Adapter
NetProfile.Name      : Xiaomi_3DAA
IPv4Address          : 192.168.0.190
IPv6DefaultGateway   : 
... output truncated ...

```

#### DNS-резолвинг

```powershell
Resolve-DnsName example.org
```

- Статус: OK
- Для чего: Проверяет, как система разрешает доменное имя.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Name                                           Type   TTL   Section    IPAddress                                
----                                           ----   ---   -------    ---------                                
example.org                                    AAAA   281   Answer     2606:4700:10::ac42:9ded                  
example.org                                    AAAA   281   Answer     2606:4700:10::6814:1a88                  
example.org                                    A      183   Answer     104.20.26.136                            
example.org                                    A      183   Answer     172.66.157.237                           



```

### [Windows] Диски и файловые системы

#### Свободное место на томах

```powershell
Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size
```

- Статус: OK
- Для чего: Показывает свободное место на томах и помогает быстро найти переполненный раздел.
- Примечание: Команда выполнена через локальный PowerShell.

```text

DriveLetter FileSystemLabel            SizeRemaining         Size
----------- ---------------            -------------         ----
          C                             338556690432 510351372288
                                            58380288    815788032
          E Microsoft Office 2016-2019             0   2341076992
                                            89751552    817885184



```

#### Файловые диски PowerShell

```powershell
Get-PSDrive -PSProvider FileSystem
```

- Статус: OK
- Для чего: Показывает файловые диски в удобном для PowerShell виде.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Name           Used (GB)     Free (GB) Provider      Root                                               CurrentLocation
----           ---------     --------- --------      ----                                               ---------------
C                 160,00        315,31 FileSystem    C:\                                 ...esktop\проекты\безопасность
E                   2,18          0,00 FileSystem    E:\                                                               



```

### [Windows] Процессы и производительность

#### Тяжёлые процессы по CPU

```powershell
Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS
```

- Статус: OK
- Для чего: Помогает быстро найти процессы, которые съели больше всего процессорного времени.
- Примечание: Команда выполнена через локальный PowerShell.

```text

ProcessName          CPU        WS
-----------          ---        --
browser     13134,765625 367923200
browser         5504,125 194174976
browser       3273,09375 292937728
Code          884,171875 261840896
browser              805  24858624
Code           413,15625 128831488
browser       339,171875  54251520
Code           305,46875 129458176
browser       297,328125 377896960
Codex         257,359375 732655616



```

#### Свободная память

```powershell
Get-Counter '\Memory\Available MBytes'
```

- Статус: OK
- Для чего: Показывает, сколько памяти сейчас доступно системе.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Timestamp                  CounterSamples                                                                              
---------                  --------------                                                                              
19.04.2026 18:39:17        \\paravoc\memory\available mbytes :                                                         
                           4861                                                                                        
                                                                                                                       
                                                                                                                       



```

### [Windows] Службы и автозапуск

#### Активные службы

```powershell
Get-Service | Where-Object Status -eq 'Running' | Select-Object -First 20 Name,DisplayName,Status
```

- Статус: OK
- Для чего: Показывает работающие службы и помогает быстро понять профиль хоста.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Name                        DisplayName                                                    Status
----                        -----------                                                    ------
AMD Crash Defender Service  AMD Crash Defender Service                                    Running
AMD External Events Utility AMD External Events Utility                                   Running
AppIDSvc                    Удостоверение приложения                                      Running
Appinfo                     Сведения о приложении                                         Running
AudioEndpointBuilder        Средство построения конечных точек Windows Audio              Running
Audiosrv                    Windows Audio                                                 Running
BDESVC                      Служба шифрования дисков BitLocker                            Running
BFE                         Служба базовой фильтрации                                     Running
BrokerInfrastructure        Служба инфраструктуры фоновых задач                           Running
BTAGService                 Служба звукового шлюза Bluetooth                              Running
BthAvctpSvc                 Служба AVCTP                                                  Running
bthserv                     Служба поддержки Bluetooth                                    Running
camsvc                      Служба диспетчера доступа к возможностям                      Running
cbdhsvc_1efab6b8            Пользовательская служба буфера обмена_1efab6b8                Running
CDPSvc                      Служба платформы подключенных устройств                       Running
... output truncated ...

```

#### Активные задачи планировщика

```powershell
Get-ScheduledTask | Where-Object State -ne 'Disabled' | Select-Object -First 20 TaskName,TaskPath,State
```

- Статус: OK
- Для чего: Показывает включенные scheduled tasks и помогает искать подозрительный автозапуск.
- Примечание: Команда выполнена через локальный PowerShell.

```text

TaskName                                          TaskPath                                                             
--------                                          --------                                                             
CreateExplorerShellUnelevatedTask                 \                                                                    
UpdateTorrent                                     \                                                                    
Обновление Браузера Яндекс                        \                                                                    
Office Automatic Updates 2.0                      \Microsoft\Office\                                                   
Office ClickToRun Service Monitor                 \Microsoft\Office\                                                   
Office Feature Updates                            \Microsoft\Office\                                                   
Office Feature Updates Logon                      \Microsoft\Office\                                                   
OfficeTelemetryAgentFallBack2016                  \Microsoft\Office\                                                   
OfficeTelemetryAgentLogOn2016                     \Microsoft\Office\                                                   
BackgroundDownload                                \Microsoft\VisualStudio\Updates\                                     
.NET Framework NGEN v4.0.30319                    \Microsoft\Windows\.NET Framework\                                   
.NET Framework NGEN v4.0.30319 64                 \Microsoft\Windows\.NET Framework\                                   
RecoverabilityToastTask                           \Microsoft\Windows\AccountHealth\                                    
AD RMS Rights Policy Template Management (Manual) \Microsoft\Windows\Active Directory Rights Management Services Cli...
PolicyConverter                                   \Microsoft\Windows\AppID\                                            
... output truncated ...

```

### [Windows] Журналы и защита

#### Состояние Defender

```powershell
Get-MpComputerStatus
```

- Статус: OK
- Для чего: Показывает состояние Microsoft Defender, сигнатур и real-time protection.
- Примечание: Команда выполнена через локальный PowerShell.

```text


AMEngineVersion                  : 1.1.26030.3008
AMProductVersion                 : 4.18.26030.3011
AMRunningMode                    : Normal
AMServiceEnabled                 : True
AMServiceVersion                 : 4.18.26030.3011
AntispywareEnabled               : True
AntispywareSignatureAge          : 0
AntispywareSignatureLastUpdated  : 19.04.2026 8:15:53
AntispywareSignatureVersion      : 1.449.186.0
AntivirusEnabled                 : True
AntivirusSignatureAge            : 0
AntivirusSignatureLastUpdated    : 19.04.2026 8:15:53
AntivirusSignatureVersion        : 1.449.186.0
BehaviorMonitorEnabled           : True
ComputerID                       : 83ED4B00-C455-4C07-9B65-34771679FF1E
ComputerState                    : 0
... output truncated ...

```

#### Профили firewall

```powershell
Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction
```

- Статус: OK
- Для чего: Показывает, включены ли профили Domain, Private и Public и как они обрабатывают трафик.
- Примечание: Команда выполнена через локальный PowerShell.

```text

Name    Enabled DefaultInboundAction DefaultOutboundAction
----    ------- -------------------- ---------------------
Domain     True        NotConfigured         NotConfigured
Private    True        NotConfigured         NotConfigured
Public     True        NotConfigured         NotConfigured



```

## Где искать артефакты

### [Windows] Учетные записи и доступ

#### Локальные пользователи и группы

- Где искать:
- lusrmgr.msc
- Get-LocalUser
- Get-LocalGroupMember -Group Administrators
- Команда проверки: `Get-LocalUser | Format-Table Name,Enabled,LastLogon`
- Зачем смотреть: Здесь проверяют локальные учетные записи, состояние аккаунтов и состав администраторов.

### [Windows] Журналы и события

#### Журнал безопасности Windows

- Где искать:
- Просмотр событий -> Windows Logs -> Security
- wevtutil el
- Get-WinEvent -LogName Security
- Команда проверки: `Get-WinEvent -LogName Security -MaxEvents 20`
- Зачем смотреть: Журнал Security нужен для разбора входов, блокировок, смены привилегий и другой активности безопасности.

### [Windows] Сеть и удаленный доступ

#### Профили и лог межсетевого экрана

- Где искать:
- Get-NetFirewallProfile
- %systemroot%\system32\logfiles\firewall\pfirewall.log
- Команда проверки: `Get-NetFirewallProfile; Get-Content $env:SystemRoot\System32\LogFiles\Firewall\pfirewall.log -Tail 50`
- Зачем смотреть: Здесь видно, включены ли профили firewall и что именно журналируется по сети.

### [Windows] Автозагрузка и запуск

#### Run keys и автозапуск пользователя

- Где искать:
- HKLM\Software\Microsoft\Windows\CurrentVersion\Run
- HKCU\Software\Microsoft\Windows\CurrentVersion\Run
- %APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup
- Команда проверки: `Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Run','HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'`
- Зачем смотреть: Отсюда удобно проверять классические точки автозапуска и нежелательные пользовательские записи.

### [Windows] Службы и планировщик

#### Службы Windows

- Где искать:
- services.msc
- HKLM\SYSTEM\CurrentControlSet\Services
- Get-Service
- Команда проверки: `Get-Service | Sort-Object Status,DisplayName`
- Зачем смотреть: Здесь смотрят состояние служб, тип запуска и неожиданные сервисы удаленного доступа.

#### Планировщик задач

- Где искать:
- taskschd.msc
- C:\Windows\System32\Tasks
- Get-ScheduledTask
- Команда проверки: `Get-ScheduledTask | Where-Object State -ne 'Disabled'`
- Зачем смотреть: Полезно для поиска кастомных задач, скрытой автоматизации и нежелательных путей запуска.

### [Windows] PowerShell и администрирование

#### История PowerShell

- Где искать:
- %APPDATA%\Microsoft\Windows\PowerShell\PSReadLine\ConsoleHost_history.txt
- %USERPROFILE%\Documents\WindowsPowerShell
- Команда проверки: `Get-Content (Join-Path $env:APPDATA 'Microsoft\Windows\PowerShell\PSReadLine\ConsoleHost_history.txt') -Tail 50`
- Зачем смотреть: История команд помогает понять, что запускалось на машине, но смотреть ее нужно только в рамках легитимной диагностики.

### [Windows] Временные данные и кэш

#### Temp и корзина

- Где искать:
- %TEMP%
- C:\Windows\Temp
- C:\$Recycle.Bin
- Команда проверки: `Get-ChildItem $env:TEMP -Force | Select-Object -First 20`
- Зачем смотреть: Здесь находятся временные файлы, пользовательский мусор и безопасные цели для controlled cleanup.

### [Linux] Журналы и аутентификация

#### Аутентификация и SSH

- Где искать:
- /var/log/auth.log
- journalctl -u ssh
- /var/log/secure
- Команда проверки: `sudo journalctl -u ssh -n 50`
- Зачем смотреть: Эти источники используют для проверки входов, brute-force и проблем с удаленным доступом.

### [Linux] Службы и автозапуск

#### systemd и cron

- Где искать:
- /etc/systemd/system
- /usr/lib/systemd/system
- /etc/crontab
- /etc/cron.d
- crontab -l
- Команда проверки: `systemctl list-unit-files --type=service; crontab -l`
- Зачем смотреть: Здесь видны системные сервисы, кастомные unit-файлы и регулярные задания.

### [Linux] Сеть и маршрутизация

#### Сетевые настройки и резолвинг

- Где искать:
- ip addr show
- ip route show
- /etc/hosts
- /etc/resolv.conf
- Команда проверки: `ip addr show && ip route show`
- Зачем смотреть: Этот набор помогает быстро понять адреса, маршруты, DNS и локальные переопределения имен.

### [Linux] Права и пользователи

#### Пользователи, sudo и sudoers

- Где искать:
- /etc/passwd
- /etc/group
- /etc/sudoers
- /etc/sudoers.d
- Команда проверки: `getent passwd | cut -d: -f1,7; sudo -l`
- Зачем смотреть: Здесь проверяют интерактивные аккаунты, оболочки и правила привилегированного доступа.

### [Linux] Следы пользователя

#### История shell и кэш

- Где искать:
- ~/.bash_history
- ~/.zsh_history
- ~/.cache
- /tmp
- /var/tmp
- Команда проверки: `ls -la ~/.bash_history ~/.zsh_history ~/.cache /tmp /var/tmp`
- Зачем смотреть: Полезно для ориентирования в пользовательской активности, временных файлах и безопасной очистке.

## План обучения

### Системное администрирование - База Windows и инфраструктуры

#### Что изучить

- Устройство Windows: процессы, службы, драйверы, реестр, файловые системы и загрузка ОС.
- Пользователи, группы, права, UAC, NTFS ACL и локальные политики безопасности.
- Сетевые основы: TCP/IP, DNS, DHCP, NAT, VPN, SMB, RDP и WinRM.
- Практика ориентирования в Event Viewer, services.msc, taskschd.msc и реестре.

#### Практика

- Поднять тестовую Windows VM и вручную настроить локальные учетные записи, UAC и NTFS-права.
- Через PowerShell проверить сетевую конфигурацию, DNS, listening ports и firewall profiles.
- Собрать собственный checklist первичной диагностики рабочей станции.

#### Цели

- Уметь объяснить, как загружается Windows и где лежат ключевые системные настройки.
- Уметь разбирать базовые проблемы входа, прав доступа и сети без хаотичного набора команд.
- Уметь быстро собрать базовый профиль хоста для отчета или инцидента.

### Системное администрирование - PowerShell и автоматизация

#### Что изучить

- Конвейер PowerShell, объекты, модули, remoting, jobs и scheduled tasks.
- Автоматизация инвентаризации: службы, процессы, обновления, firewall и event logs.
- Безопасные скрипты, логирование, обработка ошибок и экспорт отчетов.
- Работа с CSV, JSON, Markdown и форматированным выводом для отчетности.

#### Практика

- Сделать набор скриптов для инвентаризации пользователей, сервисов и сетевых интерфейсов.
- Настроить Scheduled Task для регулярного defensive-аудита и сохранения отчетов.
- Сделать PowerShell-скрипт, который собирает сведения о Windows Update, firewall и логах в один JSON.

#### Цели

- Писать PowerShell-скрипты осознанно, а не копипастить отдельные команды.
- Автоматизировать повторяемые задачи администратора и техподдержки.
- Уметь оформлять PowerShell-утилиты как полезные внутренние инструменты.

### Системное администрирование - Сети и диагностика

#### Что изучить

- TCP/IP, ARP, маршрутизация, NAT, DNS, DHCP, MTU и базовая модель OSI/TCP-IP.
- Диагностика через ping, tracert, nslookup, Test-NetConnection, netstat и Wireshark.
- Понимание типовых проблем: DNS не резолвит, маршрут уходит не туда, порт недоступен, firewall режет трафик.
- Разница между локальной проблемой хоста, проблемой сети и проблемой удаленного сервиса.

#### Практика

- Смоделировать проблемы DNS, gateway и firewall на тестовой машине и разобрать их по шагам.
- Снять трафик в Wireshark и вручную разобрать DNS-запрос, TCP three-way handshake и TLS ClientHello.
- Собрать мини-памятку по диагностике 'не открывается сайт/сервис/порт'.

#### Цели

- Уметь уверенно отличать сетевую проблему от проблем приложения и доступа.
- Понимать, какие команды проверяют адресацию, какие маршруты, а какие доступность порта.
- Уметь объяснить базовую сетевую картину на одном хосте без внешних подсказок.

### Системное администрирование - Linux и кроссплатформенная админка

#### Что изучить

- Структура Linux: процессы, systemd, journald, package managers, users/groups, sudo и permissions.
- Диагностика через ip, ss, ping, dig, curl, journalctl и системные журналы.
- Автоматизация через Bash, cron, systemd timers и базовые shell-скрипты.
- Сравнение админских задач Windows и Linux: сервисы, журналы, права, firewall, автозапуск.

#### Практика

- Поднять Linux VM и повторить те же проверки, что и на Windows: пользователи, порты, сервисы и firewall.
- Собрать собственную шпаргалку Linux-команд для сети, логов, сервисов и прав доступа.
- Сделать таблицу соответствий: Event Viewer vs journalctl, Services vs systemd, Task Scheduler vs cron.

#### Цели

- Понимать, где на Linux смотреть сервисы, журналы, маршруты и firewall.
- Уметь сравнивать Windows- и Linux-подходы к одной и той же админской задаче.
- Уметь быстро собрать baseline Linux-хоста для hardening или диагностики.

### Системное администрирование - Обслуживание, обновления и восстановление

#### Что изучить

- Windows Update, драйверы, журналы, точки восстановления, резервное копирование и recovery.
- Мониторинг памяти, диска, автозагрузки, служб и общего состояния хоста.
- Безопасная очистка временных данных и понимание, что именно можно удалять без вреда.
- Проверка health-check: аптайм, свободное место, pending reboot, обновления, защитные службы.

#### Практика

- Собрать чек-лист обслуживания рабочей станции и сервера.
- Отработать сценарий обновления, перезагрузки и отката на тестовой VM.
- Проверить, как меняется состояние хоста до и после очистки temp и корзины.

#### Цели

- Уметь регулярно вести базовый health-check Windows-хоста.
- Быстро собирать первичную диагностику после инцидента или жалобы пользователя.
- Понимать, где профилактика, а где уже расследование или remediation.

### Информационная безопасность - Фундамент ИБ

#### Что изучить

- CIA triad, риск-менеджмент, поверхность атаки, уязвимости, hardening и defence in depth.
- Аутентификация и авторизация, MFA, least privilege, password policy и account lockout.
- Сетевые угрозы, lateral movement, persistence, privilege escalation, phishing и malware basics.
- Понимание разницы между preventive, detective и corrective controls.

#### Практика

- Сопоставить настройки локальной безопасности Windows с типовыми рисками.
- Разобрать несколько техник MITRE ATT&CK и найти их возможные следы на Windows-хосте.
- Составить небольшую матрицу рисков для рабочей станции, сервера и домашней лаборатории.

#### Цели

- Уметь читать security baseline и объяснять назначение каждого контроля.
- Отличать плохую конфигурацию от нормальной административной активности.
- Понимать, как технические настройки связаны с реальными рисками бизнеса или пользователя.

### Информационная безопасность - Windows Security Operations

#### Что изучить

- Defender, firewall, audit policy, event logs, scheduled tasks, autoruns, services и PowerShell logging.
- Индикаторы компрометации в учетках, сервисах, задачах, сетевых подключениях и журналах.
- Базовый incident response: triage, containment, evidence preservation, eradication и lessons learned.
- Разница между рутинной админской активностью и подозрительным поведением на хосте.

#### Практика

- Собрать playbook проверки хоста: аккаунты, порты, сервисы, обновления, firewall и журналы.
- Сравнить нормальные и подозрительные scheduled tasks на тестовой машине.
- Подготовить шаблон отчета по findings: что обнаружено, почему это важно, что делать дальше.

#### Цели

- Уметь провести первичный defensive-аудит Windows-машины.
- Уметь оформлять findings и рекомендации понятным техническим языком.
- Уметь защищать свои выводы ссылкой на конкретные артефакты и настройки.

### Информационная безопасность - Журналы, артефакты и форензика хоста

#### Что изучить

- Windows Security/System/Application logs, PowerShell history, Run keys, Scheduled Tasks, Services и Prefetch basics.
- Linux auth logs, journalctl, cron, shell history, package logs и systemd units.
- Понимание, какие артефакты относятся к входам, запуску ПО, сети, persistence и изменениям конфигурации.
- Аккуратная работа с артефактами: сначала чтение и фиксация, потом только изменения.

#### Практика

- Сделать собственную карту артефактов Windows и Linux для расследования инцидента на хосте.
- Разобрать одно событие входа в Windows и один вход по SSH в Linux с поиском всех связанных следов.
- Подготовить краткий runbook 'что смотреть первым делом на подозрительном хосте'.

#### Цели

- Понимать, где искать следы активности пользователя, службы, задачи и удаленного доступа.
- Уметь строить последовательность событий по нескольким разным источникам.
- Уметь не ломать доказательную картину лишними действиями на хосте.

### Информационная безопасность - Детектирование и мониторинг

#### Что изучить

- Логика detection engineering: событие, сигнал, корреляция, шум, приоритет, false positive и tuning.
- Базовые источники телеметрии: Windows Event Logs, Defender, firewall, Sysmon, PowerShell, VPN и proxy.
- Разница между алертом, наблюдением, finding и подтвержденным инцидентом.
- Минимальный набор проверок, который нужен even without SIEM: локальные журналы, сервисы, задачи, сеть.

#### Практика

- Сформулировать 10 простых детектирующих правил на человеческом языке для Windows-хоста.
- Разобрать, почему открытый порт или подозрительная задача сами по себе еще не всегда инцидент.
- Сделать таблицу 'артефакт -> какой риск он может показывать -> чем подтвердить'.

#### Цели

- Мыслить не только командами, но и гипотезами наблюдения.
- Понимать, как отличать полезный сигнал от шумного признака.
- Уметь проектировать defensive-checks даже без полноценного SIEM.

### Информационная безопасность - Linux Security Basics

#### Что изучить

- SSH, sudo, PAM, journalctl, nftables или UFW, file permissions, SUID/SGID и systemd services.
- Поиск подозрительных cron jobs, нестандартных сервисов, открытых портов и опасных прав доступа.
- Базовый hardening: обновления, SELinux или AppArmor, fail2ban и ограниченные учетные записи.
- Понимание типовых рисков на интернет-доступном Linux-хосте.

#### Практика

- Проверить Linux-хост на открытые порты, cron, sudoers, SUID binaries и журнал SSH.
- Сравнить нормальные и подозрительные systemd units на тестовой машине.
- Подготовить краткий Linux hardening checklist для VM или VPS.

#### Цели

- Уметь делать базовый defensive-review Linux-сервера без хаотичного набора команд.
- Понимать, какие Linux-артефакты чаще всего важны для hardening и расследования.
- Уметь быстро оценить, где у Linux-хоста самая слабая точка: сеть, права, сервисы или обновления.

### Информационная безопасность - Identity, AD и корпоративная среда

#### Что изучить

- Active Directory, Kerberos, NTLM, GPO, delegated administration, service accounts и secrets hygiene.
- Разделение административных ролей, tiering, jump hosts, LAPS и аудит привилегированного доступа.
- Onboarding и offboarding, inventory, patching и vulnerability management.
- Почему локальные администраторы на рабочих станциях часто становятся входной точкой для lateral movement.

#### Практика

- Подготовить схему ролей доступа для небольшой организации.
- Смоделировать безопасный жизненный цикл учетной записи сотрудника.
- Составить список контролей для рабочих станций, серверов и учетных записей администраторов.

#### Цели

- Понимать, где чаще всего ломается контроль доступа в корпоративной Windows-среде.
- Понимать, почему локальные администраторы опасны на рабочих станциях.
- Уметь объяснить бизнесу и преподавателю, зачем нужны tiering, GPO и контроль привилегий.

### Информационная безопасность - Веб, API и прикладная база

#### Что изучить

- HTTP, HTTPS, cookies, tokens, headers, reverse proxy, REST API и базовая веб-авторизация.
- Частые прикладные риски: слабая аутентификация, открытые admin-панели, утечки токенов, небезопасные API.
- Логи веб-сервера, коды ответов, rate limiting и роль reverse proxy/firewall.
- Почему специалисту по ИБ полезно понимать веб и API даже при хостовом уклоне.

#### Практика

- Разобрать запросы браузера к тестовому сайту через DevTools и понять, где проходят токены и cookies.
- Сравнить обычную пользовательскую активность в веб-приложении и подозрительную последовательность запросов.
- Сделать конспект по тому, какие журналы и метрики нужны для расследования веб-инцидента.

#### Цели

- Понимать, как хостовая безопасность связывается с безопасностью приложений и API.
- Уметь читать базовые веб-логи и не путаться в кодах ответов и типовых запросах.
- Иметь фундамент для дальнейшего роста в blue team, DevSecOps или application security.

### Информационная безопасность - Vulnerability Management и hardening

#### Что изучить

- Разница между finding, misconfiguration, vulnerability и exploit path.
- Приоритизация: что критично исправлять сразу, что можно планировать, а что принять как риск.
- Hardening Windows и Linux: отключение лишнего, уменьшение поверхности атаки, контроль удаленного доступа.
- Связь между аудитом конфигурации, патчами, сервисами и правами доступа.

#### Практика

- Сделать собственную матрицу приоритизации findings для дипломного проекта.
- Сравнить два набора настроек: удобные, но небезопасные, и более защищенные, но управляемые.
- Подготовить рекомендации в стиле: быстрые исправления, плановые исправления, мониторинг.

#### Цели

- Уметь не просто перечислять проблемы, а приоритизировать их по риску.
- Уметь давать рекомендации, которые реально можно внедрить.
- Понимать, где hardening помогает, а где уже нужен процесс мониторинга и реагирования.

### Информационная безопасность - Incident Response и мышление аналитика

#### Что изучить

- Этапы IR: подготовка, triage, containment, investigation, eradication, recovery и post-incident review.
- Формулировка гипотез: что именно подозрительно, чем это подтверждается, чего не хватает для уверенности.
- Работа с неопределенностью: не делать громких выводов без артефактов, но и не игнорировать признаки.
- Коммуникация findings: кратко, доказуемо, с понятным следующим шагом.

#### Практика

- Разобрать вымышленный инцидент на Windows-хосте и расписать пошаговый triage.
- Сделать шаблон 'что писать в отчете после первичной проверки хоста'.
- Смоделировать ситуацию, где один finding оказывается ложной тревогой, а другой требует действий.

#### Цели

- Мыслить как аналитик, а не как набор команд.
- Уметь отличать факт, интерпретацию и предположение.
- Уметь обосновать, почему конкретная проверка была важна и что она показала.

### Общее развитие - Практический стек и лаборатории

#### Что изучить

- VirtualBox или Hyper-V, Windows Sandbox, Sysinternals, Wireshark, Procmon, Autoruns и Event Viewer.
- Git, CMake, C++ для внутренних defensive tools, Markdown-отчеты, JSON-экспорт и история запусков.
- Документирование: runbooks, baselines, чек-листы и отчеты по findings.
- Организация домашней лаборатории: отдельные VM, снимки, сценарии поломок и сценарии аудита.

#### Практика

- Поднять домашний lab из нескольких VM и отрабатывать hardening и расследование.
- Сделать мини-набор утилит: инвентаризация, аудит прав, аудит firewall и анализ журналов.
- Подготовить скриншоты, сценарии запуска и демонстрационные кейсы для дипломной защиты.

#### Цели

- Не просто знать команды, а уметь строить вокруг них собственные инструменты и процессы.
- Собрать портфолио учебных defensive-проектов, пригодное для диплома и собеседований.
- Уметь показать проект как практический инструмент, а не просто набор кода.

### Общее развитие - Рост в C++ и defensive tooling

#### Что изучить

- Как писать утилиты на C++ для Windows: WinAPI, Unicode, JSON, файлы, процессы, реестр, многомодульность.
- Как проектировать defensive-инструмент: слой данных, слой модулей, сервисный слой, CLI и GUI.
- Чистый CMake, разделение .h/.cpp, тестируемость, расширяемость и аккуратная работа с системными API.
- Разница между учебной PoC-утилитой и проектом, который не стыдно показать на GitHub.

#### Практика

- Добавить в проект еще один модуль аудита по своему выбору и встроить его во все слои.
- Подготовить README со скриншотами, сценарием запуска и пояснением архитектуры.
- Сделать дорожную карту развития проекта после диплома: SQLite, autoruns, diff history, политика baseline.

#### Цели

- Уметь объяснить архитектуру проекта преподавателю и на собеседовании.
- Уметь развивать инструмент дальше без хаотичных переписываний.
- Собрать проект, который выглядит как осмысленный инженерный продукт.

### Системное администрирование - Оснастки Windows и быстрая навигация

#### Что изучить

- Что находится в меню Win+X: Event Viewer, Device Manager, Disk Management, Network Connections, Task Manager, Power Options и Terminal.
- Разница между современными Settings, классическими .cpl и MMC-оснастками .msc.
- Как быстро открывать системные разделы через Win+R: eventvwr.msc, devmgmt.msc, diskmgmt.msc, ncpa.cpl, powercfg.cpl.
- Как связывать GUI-путь и PowerShell-команду для одной и той же проверки.

#### Практика

- Сделать свою карту навигации: симптом -> нужная оснастка -> что там смотреть -> какая команда подтверждает вывод.
- Пройтись по каждому пункту Win+X на тестовой машине и коротко записать, для чего он нужен.
- Подготовить мини-справочник для пользователя: где смотреть сеть, диски, автозагрузку, ошибки и питание.

#### Цели

- Уметь быстро выбрать правильную оснастку под проблему, а не открывать всё подряд.
- Понимать, когда удобнее GUI, а когда быстрее терминал.
- Объяснять путь до нужной настройки человеку без технического бэкграунда.

### Системное администрирование - Оптимизация, автозагрузка и обслуживание Windows

#### Что изучить

- Процессы, память, диск, сетевую активность и вкладки Task Manager для первичной оптимизации.
- Автозагрузка через Task Manager, Win32_StartupCommand, Run keys и scheduled tasks.
- Питание и производительность: power plans, battery report, sleep, hibernation и wake sources.
- Безопасная очистка temp, кэшей, корзины и диагностика нехватки места на системном томе.

#### Практика

- Сравнить состояние системы до и после отключения лишней автозагрузки на тестовой VM.
- Сделать чек-лист оптимизации: процессы, службы, автозапуск, свободное место, обновления, питание.
- Разобрать один сценарий 'компьютер тормозит' и один сценарий 'ноутбук быстро разряжается'.

#### Цели

- Уметь отличать полезную оптимизацию от вредного шаманства.
- Понимать, где проблема в ресурсах, а где в драйверах, обновлениях или сети.
- Давать пользователю практичные рекомендации без опасных отключений системных функций.

