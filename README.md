# Windows Security Workbench

`Windows Security Workbench` — учебное defensive-приложение на `C++23 + CMake` для изучения:

- системного администрирования Windows;
- базовой Linux-админки;
- практических команд PowerShell и Linux shell;
- хостовой информационной безопасности;
- диагностики сети, дисков, служб, автозагрузки, журналов и доступа.

Проект сделан как настольный GUI-инструмент и отдельный CLI, чтобы им было удобно пользоваться и как учебником, и как практической утилитой.

## Что умеет

- defensive-аудит Windows-хоста;
- безопасная очистка временных файлов и корзины с `dry-run`;
- большой каталог команд по Windows и Linux;
- практические сценарии вида `симптом -> что проверить -> какими командами -> как интерпретировать`;
- запуск безопасных read-only команд прямо внутри приложения;
- live-проверки Windows из PowerShell;
- live-проверки Linux через `WSL`, если в системе установлен дистрибутив;
- справочник `где что лежит` по системным артефактам;
- встроенный roadmap по системному администрированию и ИБ;
- экспорт результатов в `Markdown` и `JSON`;
- локальную историю запусков.

## Для кого проект

Проект подходит, если ты хочешь:

- запоминать команды не абстрактно, а через реальные сценарии;
- быстро понять, куда смотреть в Windows: `Win+X`, `Event Viewer`, `Disk Management`, `Task Manager`, `Network Connections` и т.д.;
- видеть рядом и Windows-команду, и Linux-аналог;
- собрать нормальный дипломный или pet-проект, который не стыдно положить на GitHub.

## Что внутри

Приложение построено вокруг нескольких учебных слоев:

1. `Команды`
   точные команды, назначение, пример вывода, как читать результат, что проверить потом.

2. `Практика`
   готовые диагностические плейбуки по категориям:
   `сеть`, `диски`, `процессы`, `службы`, `автозагрузка`, `журналы`, `доступ`.

3. `Live-проверки`
   безопасное выполнение read-only команд внутри приложения.

4. `Где искать`
   карта артефактов и системных мест:
   реестр, журналы, задачи, службы, startup locations, firewall logs и т.д.

5. `Обучение`
   roadmap по Windows admin, Linux basics, hardening, логам, сетям, incident response и defensive tooling.

## Сценарии

CLI и GUI работают с одними и теми же сценариями:

- `audit` — только defensive-аудит Windows;
- `cleanup` — очистка кэша и временных файлов;
- `learn` — режим учебника: команды, плейбуки, инструменты, артефакты, roadmap;
- `practice` — практический режим с live-проверками;
- `study` — только учебные разделы;
- `full` — аудит + очистка + учебные материалы.

## GUI

GUI-приложение уже включает:

- тёмную тему;
- русский интерфейс;
- быстрые сценарии в левой панели;
- вкладки:
  `Сводка`, `Находки`, `Команды`, `Практика`, `Где искать`, `Обучение`, `JSON`, `Инструменты`;
- экспорт в `Markdown` и `JSON`;
- отдельную опцию `Пробовать Linux через WSL`.

По умолчанию GUI стартует в учебном режиме, а не в агрессивном полном аудите, чтобы проект ощущался как тренажёр.

## Быстрый старт

### Запуск одним файлом

Самый удобный способ:

```powershell
.\Start-Workbench.ps1
```

Скрипт делает следующее:

- если проект ещё не сконфигурирован, вызывает `cmake -S . -B build`;
- если бинарники устарели, вызывает `cmake --build build --config Release`;
- если всё уже собрано, просто запускает приложение.

GUI запускается по умолчанию.

### CLI через тот же launcher

```powershell
.\Start-Workbench.ps1 practice --report practice.md --json practice.json
.\Start-Workbench.ps1 practice --linux-live --report practice-linux.md --json practice-linux.json
.\Start-Workbench.ps1 full --dry-run --report report.md --json report.json
```

## Ручная сборка

Требования:

- Windows 10/11;
- Visual Studio / MSVC с поддержкой `C++23`;
- `CMake >= 3.20`;
- опционально `WSL`, если хочешь live-проверки Linux-команд.

Сборка:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Запуск:

```powershell
.\build\Release\WindowsSecurityWorkbenchGUI.exe
.\build\Release\WindowsSecurityWorkbench.exe help
```

## CLI-примеры

```powershell
.\build\Release\WindowsSecurityWorkbench.exe audit --with-commands --with-study --report audit.md --json audit.json
.\build\Release\WindowsSecurityWorkbench.exe cleanup --dry-run --report cleanup.md --json cleanup.json
.\build\Release\WindowsSecurityWorkbench.exe learn --report learn.md --json learn.json
.\build\Release\WindowsSecurityWorkbench.exe practice --report practice.md --json practice.json
.\build\Release\WindowsSecurityWorkbench.exe practice --linux-live --report practice-linux.md --json practice-linux.json
.\build\Release\WindowsSecurityWorkbench.exe full --dry-run --report report.md --json report.json
```

## Структура проекта

```text
src/
  app/            CLI и orchestration сценариев
  core/           общие модели данных и утилиты
  gui/            WinAPI GUI
  history/        сохранение истории запусков
  modules/
    accounts/     аудит учётных записей
    artifacts/    карта системных артефактов
    cleanup/      безопасная очистка
    commands/     каталог команд Windows/Linux
    firewall/     аудит firewall
    lab/          live-запуск безопасных команд
    learning/     roadmap обучения
    network/      аудит сети
    playbooks/    практические сценарии
    services/     аудит служб
    system/       системный аудит Windows
    tasks/        аудит scheduled tasks
    tools/        справочник GUI-оснасток и Win+X
    updates/      аудит обновлений
  platform/windows/
  report/         Markdown и JSON экспорт
```

## Учебное покрытие

### Windows

- локальные пользователи и администраторы;
- парольная политика;
- сетевые интерфейсы, DNS, порты;
- firewall и Defender;
- службы и scheduled tasks;
- диски, тома, TEMP, автозагрузка;
- Event Viewer и системные события;
- Win+X и основные оснастки.

### Linux

- `id`, `getent`, `sudo -l`, `last`;
- `ip addr`, `ip route`, `getent hosts`, `ss`;
- `df`, `lsblk`, `findmnt`;
- `ps`, `systemctl`, `uptime`;
- `journalctl`, SSH-журналы;
- базовые сценарии hardening и диагностики.

## Экспорт и артефакты

После запусков можно получить:

- `report.md` / `report.json`;
- `learn.md` / `learn.json`;
- `study.md` / `study.json`;
- `practice.md` / `practice.json`;
- `practice-linux.md` / `practice-linux.json`;
- снимки истории в папке `history/`.

Это удобно и для самообучения, и для приложений к диплому.

## Ограничения и границы безопасности

Проект специально сделан defensive-only.

Он:

- не извлекает пароли браузеров;
- не достаёт Wi-Fi ключи, токены, хэши, секреты;
- не работает с `LSASS` и не делает credential dumping;
- не предназначен для offensive-эксплуатации или скрытого доступа.

Live-режим запускает только безопасные read-only проверки.

## Почему проект полезен для обучения

Смысл проекта не в том, чтобы просто показать список команд.

Смысл в том, чтобы связать вместе:

- симптом;
- нужную оснастку Windows;
- нужную команду;
- типичный вывод;
- интерпретацию;
- следующий шаг диагностики.

То есть это не “справочник команд”, а “объясняющий тренажёр”.

## Roadmap

Следующие логичные шаги развития:

1. Поиск и фильтрация команд по категориям и ключевым словам.
2. Карточки `симптом -> команда -> что смотреть`.
3. Выборочное выполнение одной команды по кнопке из GUI.
4. Дифф истории между запусками.
5. SQLite для локальной базы результатов.
6. Скриншоты GUI и branding для GitHub-витрины.
7. Отдельный модуль `Autoruns / Run keys / persistence review`.

## Быстрая идея для демонстрации

Для показа проекта преподавателю или на GitHub удобно использовать такой сценарий:

1. Запустить `.\Start-Workbench.ps1`
2. Открыть режим `Практика и тесты`
3. Показать категории `Сеть`, `Диски`, `Автозагрузка`, `Журналы`
4. Запустить live-проверки Windows
5. Показать экспорт в `Markdown` и `JSON`
6. Показать, что Linux-команды тоже есть в каталоге, а live-режим работает через `WSL`

---

Если хочешь развивать проект дальше, разумный следующий шаг — сделать поиск по командам, фильтр по категориям и запуск конкретной команды прямо из карточки в GUI.
#   i n f o _ s e c u r i t y _ l e a r n i n g  
 