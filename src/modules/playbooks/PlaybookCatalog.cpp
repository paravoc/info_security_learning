#include "modules/playbooks/PlaybookCatalog.h"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace {

sec::PracticePlaybook makePlaybook(std::string platform,
                                   std::string category,
                                   std::string title,
                                   std::string symptom,
                                   std::string goal,
                                   std::initializer_list<std::string> steps,
                                   std::initializer_list<std::string> commands,
                                   std::initializer_list<std::string> expectedSignals,
                                   std::initializer_list<std::string> relatedTools) {
    sec::PracticePlaybook playbook;
    playbook.platform = std::move(platform);
    playbook.category = std::move(category);
    playbook.title = std::move(title);
    playbook.symptom = std::move(symptom);
    playbook.goal = std::move(goal);
    playbook.steps = steps;
    playbook.commands = commands;
    playbook.expectedSignals = expectedSignals;
    playbook.relatedTools = relatedTools;
    return playbook;
}

}  // namespace

namespace sec {

std::vector<PracticePlaybook> PlaybookCatalog::build() const {
    return {
        makePlaybook(
            "Windows",
            "Сеть",
            "Сайт не открывается",
            "Браузер не открывает сайт, но Wi-Fi или Ethernet вроде бы подключены.",
            "Отделить проблему DNS, маршрута, локального интерфейса и удаленного порта.",
            {
                "Проверь, есть ли у активного интерфейса корректный IPv4-адрес, шлюз и DNS.",
                "Отдельно проверь резолвинг имени сайта и доступность TCP-порта 443.",
                "Если имя резолвится, но порт не открывается, ищи фильтрацию на пути или отказ сервиса.",
            },
            {
                "Get-NetIPConfiguration",
                "Resolve-DnsName example.org",
                "Test-NetConnection example.org -Port 443",
            },
            {
                "Активный интерфейс не должен висеть без gateway.",
                "DNS должен вернуть адрес, иначе проблема в резолвинге.",
                "TcpTestSucceeded=False при рабочем DNS обычно указывает на сеть, firewall или удаленный сервис.",
            },
            {
                "Win+X -> Сетевые подключения",
                "Параметры -> Сеть и Интернет",
            }),
        makePlaybook(
            "Windows",
            "Сеть",
            "Найти лишний открытый порт",
            "Нужно понять, какие сервисы слушают входящие подключения на машине.",
            "Быстро связать порт с процессом и решить, нужен ли он вообще.",
            {
                "Выведи слушающие TCP-порты и отметь сервисные порты удаленного доступа.",
                "Если видишь 445, 3389, 5985 или нестандартный порт, выясни owning process.",
                "Сопоставь процесс с ролью хоста и реши, должен ли этот сервис быть открыт.",
            },
            {
                "Get-NetTCPConnection -State Listen | Sort-Object LocalPort",
                "Get-Service | Where-Object Status -eq 'Running'",
            },
            {
                "Порт на 0.0.0.0 виден всем интерфейсам, а не только локально.",
                "127.0.0.1 обычно безопаснее, чем 0.0.0.0.",
                "Любой неизвестный listener требует привязки к процессу и назначению.",
            },
            {
                "Win+X -> Терминал (Администратор)",
                "Win+X -> Диспетчер задач",
            }),
        makePlaybook(
            "Windows",
            "Диски",
            "Заканчивается место на системном диске",
            "Windows начал обновляться с ошибками, хранилище почти заполнено или приложения тормозят.",
            "Понять, на каком томе проблема и где искать мусор или лишние данные.",
            {
                "Проверь свободное место по всем томам и отдельно системный диск.",
                "Посмотри временные каталоги и стандартные точки накопления мусора.",
                "Перед очисткой убедись, что речь не о recovery/system разделах.",
            },
            {
                "Get-Volume | Select-Object DriveLetter,FileSystemLabel,SizeRemaining,Size",
                "Get-PSDrive -PSProvider FileSystem",
                "Get-ChildItem $env:TEMP -Force | Select-Object -First 20 Name,Length,LastWriteTime",
            },
            {
                "Если на системном томе осталось меньше 10-15 процентов, жди проблем с обновлениями и кэшем.",
                "Recovery-раздел не чистят вручную без четкого понимания последствий.",
                "Большой и старый TEMP обычно говорит, что есть смысл запускать безопасную очистку.",
            },
            {
                "Win+X -> Управление дисками",
                "Параметры -> Система -> Память",
            }),
        makePlaybook(
            "Windows",
            "Процессы и производительность",
            "Компьютер долго входит в систему",
            "После логина долго открывается рабочий стол, вентиляторы шумят, система ощущается тяжелой.",
            "Разделить проблему на автозагрузку, прожорливые процессы и нехватку памяти.",
            {
                "Сначала посмотри процессы с максимальным CPU или памятью.",
                "Потом проверь автозагрузку и отличи реально нужные агенты от лишних.",
                "Если памяти мало, проверь метрику свободной памяти и ищи долгоживущий виновный процесс.",
            },
            {
                "Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 ProcessName,CPU,WS",
                "Get-CimInstance Win32_StartupCommand | Select-Object Name,Command,Location",
                "Get-Counter '\\Memory\\Available MBytes'",
            },
            {
                "CPU показывает накопленное время, а WS отражает рабочий набор памяти.",
                "Автозагрузка из AppData и user-writable путей требует отдельной проверки.",
                "Очень низкий Available MBytes обычно означает давление на память и активный paging.",
            },
            {
                "Win+X -> Диспетчер задач",
                "Win+X -> Установленные приложения",
            }),
        makePlaybook(
            "Windows",
            "Журналы и защита",
            "Нужно понять, почему система ругается на безопасность",
            "Есть предупреждения по защите, firewall или системным событиям, но неясно, куда смотреть первым.",
            "Собрать защитную картину: Defender, firewall и последние критичные системные события.",
            {
                "Проверь, включены ли защитные профили и real-time protection.",
                "Посмотри firewall-профили и базовую inbound/outbound политику.",
                "После этого прочитай последние системные события, а не только Security.",
            },
            {
                "Get-MpComputerStatus",
                "Get-NetFirewallProfile | Select-Object Name,Enabled,DefaultInboundAction,DefaultOutboundAction",
                "Get-WinEvent -LogName System -MaxEvents 8 | Select-Object TimeCreated,Id,ProviderName,LevelDisplayName,Message",
            },
            {
                "Старые сигнатуры или отключенный real-time protection требуют объяснения.",
                "Public-профиль firewall не должен быть выключен без причины.",
                "В системных событиях ищи повторяющиеся ошибки одного provider, а не единичный шум.",
            },
            {
                "Win+X -> Просмотр событий",
                "Безопасность Windows",
            }),
        makePlaybook(
            "Linux",
            "Сеть",
            "Linux-хост не видит сеть",
            "Сервис в Linux не отвечает, и нужно быстро отделить проблему интерфейса от маршрута.",
            "Проверить адреса, маршрут по умолчанию и слушающие порты сервиса.",
            {
                "Сначала посмотри адреса интерфейсов и не перепутай реальный интерфейс с docker0 или bridge.",
                "Проверь маршрут по умолчанию и имя шлюза.",
                "Если сервис должен быть доступен, выведи listeners и адреса привязки.",
            },
            {
                "ip addr show",
                "ip route",
                "ss -tulpn",
            },
            {
                "Отсутствие default route объясняет невозможность выйти наружу.",
                "127.0.0.1 и 0.0.0.0 несут разный смысл для доступности сервиса.",
                "Bridge и container-сети не должны путать основную картину хоста.",
            },
            {
                "NetworkManager / systemd-networkd",
                "journalctl -u NetworkManager",
            }),
        makePlaybook(
            "Linux",
            "Диски",
            "Заполняется корневой раздел",
            "На Linux начинаются ошибки записи, падают сервисы или обновления из-за нехватки места.",
            "Найти перегруженную файловую систему и понять, кто съел место.",
            {
                "Начни с общей картины по файловым системам.",
                "Потом проверь блочные устройства и точки монтирования.",
                "Если корень близок к 100 процентам, отдельно смотри /var/log, контейнеры и кэш пакетов.",
            },
            {
                "df -h",
                "lsblk -f",
                "du -sh /var/log/* 2>/dev/null | sort -h | tail -n 10",
            },
            {
                "tmpfs не нужно лечить как обычный диск.",
                "Системный раздел за 80-90 процентов уже требует действий.",
                "Резкий рост в /var/log часто связан с циклической ошибкой сервиса.",
            },
            {
                "lsblk / findmnt",
                "journalctl",
            }),
        makePlaybook(
            "Linux",
            "Доступ и журналы",
            "Проверить, кто логинился и какие права есть",
            "Нужно быстро разобраться в локальных учетках, sudo и истории входов.",
            "Получить минимальную картину по пользователям, sudo и недавним логинам.",
            {
                "Сначала выведи текущего пользователя и его группы.",
                "Потом проверь sudo-права и историю логинов.",
                "Если видишь неожиданный логин, переходи к SSH-логам и журналам auth.",
            },
            {
                "id",
                "sudo -l",
                "last -a | head -n 10",
            },
            {
                "Лишняя группа sudo или wheel - уже повод на ревизию доступа.",
                "Неожиданный удаленный IP в last нужно сопоставлять с SSH-журналом.",
                "Пустой вывод last возможен на свежей системе или в контейнере.",
            },
            {
                "journalctl -u ssh",
                "/var/log/auth.log",
            }),
        makePlaybook(
            "Linux",
            "Процессы и сервисы",
            "Найти тяжёлый процесс",
            "Хост тормозит, но пока неясно, виноват CPU, память или конкретный демон.",
            "Отделить нагрузку CPU, память и профиль запущенных сервисов.",
            {
                "Сначала отсортируй процессы по памяти и CPU.",
                "Потом посмотри список активных сервисов и роль машины.",
                "Если процесс непонятен, связывай его с unit, портом и владельцем.",
            },
            {
                "ps aux --sort=-%mem | head -n 10",
                "ps aux --sort=-%cpu | head -n 10",
                "systemctl --type=service --state=running --no-pager | head -n 20",
            },
            {
                "Рост памяти базы данных может быть нормой, а случайный пользовательский процесс - нет.",
                "Высокий CPU без понятной роли часто быстрее ловится через связку ps + systemctl + ss.",
                "Сервисный профиль хоста должен совпадать с тем, для чего этот сервер нужен.",
            },
            {
                "systemctl",
                "top / htop",
            }),
    };
}

}  // namespace sec
