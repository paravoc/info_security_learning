# Windows Security Workbench

Сформировано: 2026-04-19 01:17:12

Сценарий: full

## Находки аудита

### [Сведения] Система - Профиль хоста

- Имя компьютера: paravoc
- Текущий пользователь: smirn
- Версия: Windows 10.0 (build 26200)
- Аптайм: 2d 3h 29m
- ОЗУ всего: 15.34 GB, доступно: 4.77 GB

### [Норма] Система - Состояние UAC

- Контроль учетных записей (UAC): включен
- Рекомендация: Держи UAC включенным для контролируемого повышения прав.

### [Норма] Удаленный доступ - Экспозиция Remote Desktop

- Remote Desktop: выключен
- Рекомендация: Прямой экспозиции RDP не обнаружено.

### [Норма] Защита - Служба Microsoft Defender

- Состояние службы: Запущена
- Рекомендация: Продолжай контролировать сигнатуры и статус обновлений Defender.

### [Риск] Учетные записи - Локальные пользователи

- CodexSandboxOffline :: пароль не истекает
- CodexSandboxOnline :: пароль не истекает
- DefaultAccount :: отключена, пароль не обязателен, пароль не истекает
- smirn :: пароль не истекает
- WDAGUtilityAccount :: отключена
- Администратор :: отключена, пароль не истекает
- Гость :: отключена, пароль не обязателен, пароль не истекает
- Рекомендация: Требуй пароли и избегай бессрочных паролей для повседневных локальных учеток.

### [Норма] Учетные записи - Состав группы администраторов

- Администратор
- smirn
- Рекомендация: Размер группы администраторов небольшой, ею проще управлять.

### [Риск] Учетные записи - Парольная политика и блокировка

- Минимальная длина пароля: 0
- История паролей: 0
- Максимальный возраст пароля: 42d 0h 0m
- Минимальный возраст пароля: 0m
- Порог блокировки: 10
- Длительность блокировки: 10m
- Окно наблюдения: 10m
- Рекомендация: Ориентируйся минимум на 8-12 символов, историю паролей и ненулевой порог блокировки.

### [Норма] Сеть - Активные сетевые адаптеры

- Беспроводная сеть :: fe80::282:eb2a:9944:f40d, 192.168.0.190
- outline-tap0 :: fe80::b3c6:895b:61:ba06, 10.0.85.2
- Loopback Pseudo-Interface 1 :: ::1, 127.0.0.1
- Рекомендация: Проверь, что активны только ожидаемые сетевые адаптеры.

### [Риск] Сеть - Слушающие TCP-порты

- Порт 135 :: PID 1704
- Порт 445 :: PID 4
- Порт 5040 :: PID 7572
- Порт 5432 :: PID 10284
- Порт 49664 :: PID 1392
- Порт 49665 :: PID 1216
- Порт 49666 :: PID 2368
- Порт 49667 :: PID 3180
- Порт 49669 :: PID 1304
- Порт 52149 :: PID 10428
- Порт 139 :: PID 4
- Порт 62650 :: Code.exe (PID 22892)
- Порт 139 :: PID 4
- Рекомендация: На хосте слушают чувствительные порты управления или шаринга. Проверь, действительно ли они нужны.

### [Норма] Firewall - Профили межсетевого экрана

- Domain :: enabled=True, inbound=NotConfigured, outbound=NotConfigured, allowInboundRules=NotConfigured, allowLocalRules=NotConfigured
- Private :: enabled=True, inbound=NotConfigured, outbound=NotConfigured, allowInboundRules=NotConfigured, allowLocalRules=NotConfigured
- Public :: enabled=True, inbound=NotConfigured, outbound=NotConfigured, allowInboundRules=NotConfigured, allowLocalRules=NotConfigured
- Рекомендация: Профили firewall выглядят достаточно строгими.

### [Норма] Службы - Базовые защитные службы

- Base Filtering Engine :: Запущена, start=Автоматически
- Windows Firewall :: Запущена, start=Автоматически
- Microsoft Defender :: Запущена, start=Автоматически
- Windows Update :: Не запущена, start=Вручную
- Рекомендация: Ключевые защитные службы Windows выглядят нормально.

### [Риск] Службы - Удаленное администрирование и шаринг

- Remote Registry :: Не запущена, start=Отключена
- Windows Remote Management :: Не запущена, start=Вручную
- OpenSSH Server :: Служба отсутствует, start=
- Remote Desktop Services :: Не запущена, start=Вручную
- File and Printer Sharing :: Запущена, start=Автоматически
- Рекомендация: Проверь, какие службы удаленного управления и шаринга реально нужны на этом хосте.

### [Риск] Обновления - Конфигурация Windows Update

- Автоматические обновления отключены политикой: нет
- AUOptions: недоступно
- Требуется перезагрузка после обновлений: нет
- Последняя успешная установка: неизвестно
- Рекомендация: Держи автоматические обновления включенными и контролируй регулярную успешную установку патчей.

### [Норма] Обновления - Последние установленные hotfix

- KB5077869 :: 2026-04-16 :: Security Update
- KB5083769 :: 2026-04-16 :: Security Update
- KB5088467 :: 2026-04-16 :: Security Update
- KB5083532 :: 2026-04-16 :: Security Update
- KB5078674 :: 2026-04-16 :: Update
- Рекомендация: Сверь частоту установки обновлений с вашей patch-политикой.

### [Норма] Задачи - Активные не-Microsoft scheduled tasks

- \CreateExplorerShellUnelevatedTask :: state=Ready, runLevel=n/a, actions=C:\WINDOWS\explorer.exe
- \UpdateTorrent :: state=Ready, runLevel=Highest, actions="C:\Users\smirn\AppData\Roaming\utorrent\client\uTorrentClients.exe"
- \Обновление Браузера Яндекс :: state=Ready, runLevel=n/a, actions=C:\Users\smirn\AppData\Local\Yandex\YandexBrowser\Application\browser.exe
- \PowerToys\Autorun for smirn :: state=Ready, runLevel=n/a, actions=C:\Users\smirn\AppData\Local\PowerToys\PowerToys.exe
- \SoftLanding\S-1-5-21-1967146642-2446214716-554224929-1001\SoftLandingCreativeManagementTask :: state=Ready, runLevel=n/a, actions=
- Рекомендация: Количество активных не-Microsoft задач относительно небольшое.

### [Риск] Задачи - Задачи с рискованными путями запуска

- \UpdateTorrent :: state=Ready, runLevel=Highest, actions="C:\Users\smirn\AppData\Roaming\utorrent\client\uTorrentClients.exe"
- \Обновление Браузера Яндекс :: state=Ready, runLevel=n/a, actions=C:\Users\smirn\AppData\Local\Yandex\YandexBrowser\Application\browser.exe
- \PowerToys\Autorun for smirn :: state=Ready, runLevel=n/a, actions=C:\Users\smirn\AppData\Local\PowerToys\PowerToys.exe
- Рекомендация: Задачи, запускающие исполняемые файлы из user-writable путей, требуют ручной проверки.

## Очистка и кэш

- Режим: dry-run
- Пользовательский temp (C:\Users\smirn\AppData\Local\Temp\) | files=1498 | bytes=1.32 GB | message=Только dry-run. Файлы не удалялись.
- Системный temp (C:\WINDOWS\Temp) | files=0 | bytes=0 B | message=Только dry-run. Файлы не удалялись.
- Корзина | files=97 | bytes=24.87 GB | message=Только dry-run. Корзина не очищалась.

## Команды и шпаргалки

### [Windows] Учетные записи и доступ

#### Локальные пользователи

```powershell
Get-LocalUser
```

- Назначение: Показывает локальные учетные записи Windows, их состояние и базовые свойства.
- Осторожно: Команда только читает данные. Используй ее вместо попыток доставать пароли или секреты.

#### Парольная политика

```powershell
net accounts
```

- Назначение: Показывает минимальную длину пароля, срок действия и параметры блокировки.
- Осторожно: На управляемых ПК часть данных может быть полезнее сверять еще и через локальные политики.

#### Локальные администраторы

```powershell
Get-LocalGroupMember -Group Administrators
```

- Назначение: Показывает, кто сейчас входит в группу локальных администраторов.
- Осторожно: Сначала проверяй необходимость прав, а не меняй состав группы вслепую.

### [Windows] Сеть

#### IP-конфигурация

```powershell
Get-NetIPConfiguration
```

- Назначение: Показывает сетевые адаптеры, IP-адреса, шлюзы и DNS.
- Осторожно: Хорошая базовая команда для ориентирования и диагностики.

#### Слушающие TCP-порты

```powershell
Get-NetTCPConnection -State Listen
```

- Назначение: Показывает процессы, которые слушают входящие TCP-порты.
- Осторожно: Особенно внимательно смотри на 3389, 445, 5985 и 5986.

#### Проверка удаленного подключения

```powershell
Test-NetConnection example.org -Port 443
```

- Назначение: Проверяет DNS-резолвинг и доступность удаленной точки по TCP.
- Осторожно: Годится для диагностики. Не используй это как сканер чужих сетей.

### [Windows] Защита

#### Службы безопасности

```powershell
Get-Service WinDefend, MpsSvc, wuauserv, WinRM, RemoteRegistry, sshd
```

- Назначение: Быстро проверяет ключевые защитные и удаленные сервисы.
- Осторожно: Не отключай сервисы до понимания их роли на конкретной машине.

#### Состояние Defender

```powershell
Get-MpComputerStatus
```

- Назначение: Показывает статус Microsoft Defender, сигнатур и защиты в реальном времени.
- Осторожно: Если Defender отсутствует, проверь, не стоит ли другой endpoint protection.

#### Профили firewall

```powershell
Get-NetFirewallProfile
```

- Назначение: Показывает, включены ли профили Domain, Private и Public и как они настроены.
- Осторожно: Смотри на выключенные профили и входящее действие Allow.

#### Разрешающие inbound-правила

```powershell
Get-NetFirewallRule -Enabled True -Direction Inbound -Action Allow
```

- Назначение: Показывает активные входящие правила, открывающие сервисы наружу.
- Осторожно: Проверь бизнес-необходимость каждого правила управления и шаринга.

### [Windows] Журналы и следы

#### Последние события Security

```powershell
Get-WinEvent -LogName Security -MaxEvents 20
```

- Назначение: Показывает свежие события журнала безопасности.
- Осторожно: Чтение Security log может требовать повышенных прав и включенного аудита.

#### Инвентарь журналов

```powershell
Get-WinEvent -ListLog * | Sort-Object RecordCount -Descending | Select-Object -First 20
```

- Назначение: Помогает понять, какие журналы активны и где больше всего событий.
- Осторожно: Удобно использовать как первый шаг перед более глубоким разбором.

### [Windows] Службы и автозапуск

#### Активные службы

```powershell
Get-Service | Sort-Object Status, DisplayName
```

- Назначение: Показывает состояние служб и помогает заметить неожиданные сервисы.
- Осторожно: Фокусируйся на защите, удаленном доступе, резервном копировании и агентах администрирования.

#### Активные scheduled tasks

```powershell
Get-ScheduledTask | Where-Object State -ne 'Disabled'
```

- Назначение: Показывает активные задачи планировщика.
- Осторожно: Перед отключением задач проверь, не связаны ли они с обновлениями, драйверами или агентами.

#### Run keys

```powershell
Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Run','HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
```

- Назначение: Показывает классические точки автозапуска в HKLM и HKCU.
- Осторожно: Особенно внимательно проверяй пути из AppData, Temp и других user-writable каталогов.

### [Windows] Файлы и права

#### ACL объекта

```powershell
Get-Acl C:\Path\To\Item | Format-List
```

- Назначение: Показывает права доступа к файлу или папке.
- Осторожно: Нужна для разбора access denied и чрезмерно широких разрешений.

#### SMB-шары

```powershell
Get-SmbShare
```

- Назначение: Показывает локальные SMB-шары и помогает оценить поверхность файлового шаринга.
- Осторожно: Шары должны быть обоснованы и защищены NTFS- и share-permissions.

### [Windows] Очистка и кэш

#### Сброс DNS-кэша

```powershell
ipconfig /flushdns
```

- Назначение: Очищает локальный кэш DNS-резолвера при диагностике проблем с именами.
- Осторожно: Это средство диагностики, а не очистка от вредоносного ПО.

### [Linux] Сеть

#### Сетевые интерфейсы

```bash
ip addr show
```

- Назначение: Показывает интерфейсы Linux и назначенные им IP-адреса.
- Осторожно: Современная замена ifconfig на большинстве дистрибутивов.

#### Маршруты

```bash
ip route show
```

- Назначение: Показывает активную таблицу маршрутизации.
- Осторожно: Полезно при разборе VPN, шлюзов и асимметричных маршрутов.

#### Слушающие порты

```bash
ss -tulpn
```

- Назначение: Показывает TCP/UDP listeners вместе с owning processes.
- Осторожно: Предпочтительнее старого netstat на современных системах.

### [Linux] Службы и защита

#### Активные сервисы

```bash
systemctl --type=service --state=running
```

- Назначение: Показывает запущенные systemd-сервисы.
- Осторожно: Обращай внимание на неожиданные веб-сервисы, базы и удаленный доступ.

#### Правила nftables

```bash
sudo nft list ruleset
```

- Назначение: Показывает активный ruleset nftables.
- Осторожно: На старых системах вместо этого может использоваться iptables или ufw.

#### Статус UFW

```bash
sudo ufw status verbose
```

- Назначение: Показывает, включен ли UFW и какие правила действуют.
- Осторожно: Полезно на Ubuntu-подобных системах.

### [Linux] Журналы и следы

#### Журнал SSH

```bash
sudo journalctl -u ssh -n 50
```

- Назначение: Показывает свежие события, связанные с SSH.
- Осторожно: Подходит для проверки логинов и brute-force попыток.

#### Системный журнал

```bash
sudo journalctl -xe
```

- Назначение: Показывает свежие системные события с дополнительным контекстом.
- Осторожно: Не запускай без фильтра на перегруженных production-хостах во время инцидента.

### [Linux] Службы и автозапуск

#### Cron текущего пользователя

```bash
crontab -l
```

- Назначение: Показывает задания cron для текущего пользователя.
- Осторожно: Дополнительно смотри /etc/crontab и /etc/cron.* для системных задач.

### [Linux] Учетные записи и доступ

#### История входов

```bash
last -a
```

- Назначение: Показывает недавние логины и IP-адреса.
- Осторожно: Полезно вместе с lastb, если включен учет неудачных входов.

### [Linux] Файлы и права

#### Права и ACL

```bash
ls -la /path && getfacl /path
```

- Назначение: Показывает UNIX-права и ACL для файлов и каталогов.
- Осторожно: Ключевая команда для поиска чрезмерно широких прав.

#### SUID-файлы

```bash
find / -perm -4000 -type f 2>/dev/null
```

- Назначение: Показывает бинарники с выставленным SUID-битом.
- Осторожно: Нужна для hardening и privilege-escalation review; на больших системах может быть медленной.

### [Linux] Процессы

#### Тяжелые процессы по памяти

```bash
ps aux --sort=-%mem | head
```

- Назначение: Показывает процессы с большим потреблением памяти.
- Осторожно: Для live-наблюдения удобнее top или htop.

#### Открытые файлы процесса

```bash
sudo lsof -p <pid>
```

- Назначение: Показывает файлы и сокеты, открытые процессом.
- Осторожно: Очень полезно в расследованиях, но часто требует повышенных прав.

### [Linux] Обновления и hardening

#### Доступные обновления

```bash
apt list --upgradable
```

- Назначение: Показывает пакеты с доступными обновлениями на Debian/Ubuntu.
- Осторожно: На других дистрибутивах используй родной пакетный менеджер.

#### Режим SELinux

```bash
getenforce
```

- Назначение: Показывает, работает ли SELinux в режиме Enforcing, Permissive или Disabled.
- Осторожно: Это базовая проверка на Fedora/RHEL-подобных системах.

#### Статус fail2ban

```bash
sudo fail2ban-client status
```

- Назначение: Показывает, запущен ли fail2ban и какие jail активны.
- Осторожно: Полезно на Linux-хостах, доступных из интернета.

### [Linux] Учетные записи и доступ

#### Пользователи и shell

```bash
getent passwd | cut -d: -f1,7
```

- Назначение: Показывает учетные записи и их login shell.
- Осторожно: Ищи неожиданные интерактивные аккаунты и нетипичные оболочки.

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

#### Практика

- Поднять тестовую Windows VM и вручную настроить локальные учетные записи, UAC и NTFS-права.
- Через PowerShell проверить сетевую конфигурацию, DNS, listening ports и firewall profiles.

#### Цели

- Уметь объяснить, как загружается Windows и где лежат ключевые системные настройки.
- Уметь разбирать базовые проблемы входа, прав доступа и сети без хаотичного набора команд.

### Системное администрирование - PowerShell и автоматизация

#### Что изучить

- Конвейер PowerShell, объекты, модули, remoting, jobs и scheduled tasks.
- Автоматизация инвентаризации: службы, процессы, обновления, firewall и event logs.
- Безопасные скрипты, логирование, обработка ошибок и экспорт отчетов.

#### Практика

- Сделать набор скриптов для инвентаризации пользователей, сервисов и сетевых интерфейсов.
- Настроить Scheduled Task для регулярного defensive-аудита и сохранения отчетов.

#### Цели

- Писать PowerShell-скрипты осознанно, а не копипастить отдельные команды.
- Автоматизировать повторяемые задачи администратора и техподдержки.

### Системное администрирование - Linux и кроссплатформенная админка

#### Что изучить

- Структура Linux: процессы, systemd, journald, package managers, users/groups, sudo и permissions.
- Диагностика через ip, ss, ping, dig, curl, journalctl и системные журналы.
- Автоматизация через Bash, cron, systemd timers и базовые shell-скрипты.

#### Практика

- Поднять Linux VM и повторить те же проверки, что и на Windows: пользователи, порты, сервисы и firewall.
- Собрать собственную шпаргалку Linux-команд для сети, логов, сервисов и прав доступа.

#### Цели

- Понимать, где на Linux смотреть сервисы, журналы, маршруты и firewall.
- Уметь сравнивать Windows- и Linux-подходы к одной и той же админской задаче.

### Системное администрирование - Обслуживание, обновления и восстановление

#### Что изучить

- Windows Update, драйверы, журналы, точки восстановления, резервное копирование и recovery.
- Мониторинг памяти, диска, автозагрузки, служб и общего состояния хоста.
- Безопасная очистка временных данных и понимание, что именно можно удалять без вреда.

#### Практика

- Собрать чек-лист обслуживания рабочей станции и сервера.
- Отработать сценарий обновления, перезагрузки и отката на тестовой VM.

#### Цели

- Уметь регулярно вести базовый health-check Windows-хоста.
- Быстро собирать первичную диагностику после инцидента или жалобы пользователя.

### Информационная безопасность - Фундамент ИБ

#### Что изучить

- CIA triad, риск-менеджмент, поверхность атаки, уязвимости, hardening и defence in depth.
- Аутентификация и авторизация, MFA, least privilege, password policy и account lockout.
- Сетевые угрозы, lateral movement, persistence, privilege escalation, phishing и malware basics.

#### Практика

- Сопоставить настройки локальной безопасности Windows с типовыми рисками.
- Разобрать несколько техник MITRE ATT&CK и найти их возможные следы на Windows-хосте.

#### Цели

- Уметь читать security baseline и объяснять назначение каждого контроля.
- Отличать плохую конфигурацию от нормальной административной активности.

### Информационная безопасность - Windows Security Operations

#### Что изучить

- Defender, firewall, audit policy, event logs, scheduled tasks, autoruns, services и PowerShell logging.
- Индикаторы компрометации в учетках, сервисах, задачах, сетевых подключениях и журналах.
- Базовый incident response: triage, containment, evidence preservation, eradication и lessons learned.

#### Практика

- Собрать playbook проверки хоста: аккаунты, порты, сервисы, обновления, firewall и журналы.
- Сравнить нормальные и подозрительные scheduled tasks на тестовой машине.

#### Цели

- Уметь провести первичный defensive-аудит Windows-машины.
- Уметь оформлять findings и рекомендации понятным техническим языком.

### Информационная безопасность - Linux Security Basics

#### Что изучить

- SSH, sudo, PAM, journalctl, nftables или UFW, file permissions, SUID/SGID и systemd services.
- Поиск подозрительных cron jobs, нестандартных сервисов, открытых портов и опасных прав доступа.
- Базовый hardening: обновления, SELinux или AppArmor, fail2ban и ограниченные учетные записи.

#### Практика

- Проверить Linux-хост на открытые порты, cron, sudoers, SUID binaries и журнал SSH.
- Сравнить нормальные и подозрительные systemd units на тестовой машине.

#### Цели

- Уметь делать базовый defensive-review Linux-сервера без хаотичного набора команд.
- Понимать, какие Linux-артефакты чаще всего важны для hardening и расследования.

### Информационная безопасность - Identity, AD и корпоративная среда

#### Что изучить

- Active Directory, Kerberos, NTLM, GPO, delegated administration, service accounts и secrets hygiene.
- Разделение административных ролей, tiering, jump hosts, LAPS и аудит привилегированного доступа.
- Onboarding и offboarding, inventory, patching и vulnerability management.

#### Практика

- Подготовить схему ролей доступа для небольшой организации.
- Смоделировать безопасный жизненный цикл учетной записи сотрудника.

#### Цели

- Понимать, где чаще всего ломается контроль доступа в корпоративной Windows-среде.
- Понимать, почему локальные администраторы опасны на рабочих станциях.

### Общее развитие - Практический стек и лаборатории

#### Что изучить

- VirtualBox или Hyper-V, Windows Sandbox, Sysinternals, Wireshark, Procmon, Autoruns и Event Viewer.
- Git, CMake, C++ для внутренних defensive tools, Markdown-отчеты, JSON-экспорт и история запусков.
- Документирование: runbooks, baselines, чек-листы и отчеты по findings.

#### Практика

- Поднять домашний lab из нескольких VM и отрабатывать hardening и расследование.
- Сделать мини-набор утилит: инвентаризация, аудит прав, аудит firewall и анализ журналов.

#### Цели

- Не просто знать команды, а уметь строить вокруг них собственные инструменты и процессы.
- Собрать портфолио учебных defensive-проектов, пригодное для диплома и собеседований.

