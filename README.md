# Windows Security Workbench

`Windows Security Workbench` is a defensive учебный проект на `C++23 + CMake` для изучения системного администрирования Windows, базовой Linux-админки, практических PowerShell/Linux-команд и хостовой информационной безопасности.

Проект состоит из двух точек входа:

- `WindowsSecurityWorkbenchGUI.exe` — настольный GUI-тренажёр.
- `WindowsSecurityWorkbench.exe` — CLI для сценариев аудита, практики и экспорта отчётов.

Инструмент специально сделан как defensive-only: он не извлекает пароли, токены, Wi-Fi ключи, хэши и другие секреты, не работает с `LSASS` и не предназначен для offensive-задач.

## Возможности

- defensive-аудит Windows-хоста;
- безопасная очистка временных файлов и корзины с `dry-run`;
- каталог команд по Windows и Linux с пояснениями;
- практические сценарии формата `симптом -> что проверить -> чем проверить -> как интерпретировать`;
- запуск безопасных read-only команд прямо внутри приложения;
- live-проверки Windows через PowerShell;
- live-проверки Linux через `WSL`, если установлен дистрибутив;
- справочник по системным артефактам, оснасткам и точкам поиска;
- встроенный roadmap по системному администрированию и defensive security;
- экспорт результатов в `Markdown` и `JSON`;
- локальная история запусков.

## Для кого проект

Проект подойдёт, если нужен учебный инструмент, который помогает:

- учить команды через реальные сценарии, а не через голый список;
- быстро ориентироваться в ключевых местах Windows: `Event Viewer`, `Task Scheduler`, `Services`, `Disk Management`, `Win+X` и других;
- видеть рядом Windows-команды и Linux-аналоги;
- показать преподавателю или выложить на GitHub аккуратный дипломный/pet-проект с практической ценностью.

## Быстрый старт

### Запуск одним скриптом

По умолчанию launcher собирает проект при необходимости и запускает GUI:

```powershell
.\Start-Workbench.ps1
```

Что делает скрипт:

1. Проверяет наличие `build/CMakeCache.txt`.
2. При необходимости вызывает `cmake -S . -B build`.
3. Если исходники новее бинарников, вызывает `cmake --build build --config Release`.
4. Запускает `WindowsSecurityWorkbenchGUI.exe`.

### CLI через тот же launcher

```powershell
.\Start-Workbench.ps1 practice --report practice.md --json practice.json
.\Start-Workbench.ps1 practice --linux-live --report practice-linux.md --json practice-linux.json
.\Start-Workbench.ps1 full --dry-run --report report.md --json report.json
```

## Ручная сборка на Windows

### Требования

- Windows 10/11;
- Visual Studio 2022 или другой MSVC toolchain с поддержкой `C++23`;
- `CMake >= 3.20`;
- опционально `WSL`, если нужны live-проверки Linux-команд.

### Сборка

```powershell
cmake -S . -B build
cmake --build build --config Release
```

### Запуск

GUI:

```powershell
.\build\Release\WindowsSecurityWorkbenchGUI.exe
```

CLI:

```powershell
.\build\Release\WindowsSecurityWorkbench.exe help
```

## Сценарии CLI

Поддерживаются следующие режимы:

- `audit` — defensive-аудит Windows;
- `cleanup` — безопасная очистка временных файлов и корзины;
- `learn` — учебный режим: команды, плейбуки, инструменты, артефакты, roadmap;
- `study` — только учебные материалы;
- `practice` — практический режим с live-проверками;
- `full` — аудит + очистка + учебные разделы.

Примеры:

```powershell
.\build\Release\WindowsSecurityWorkbench.exe audit --with-commands --with-study --report audit.md --json audit.json
.\build\Release\WindowsSecurityWorkbench.exe cleanup --dry-run --report cleanup.md --json cleanup.json
.\build\Release\WindowsSecurityWorkbench.exe learn --report learn.md --json learn.json
.\build\Release\WindowsSecurityWorkbench.exe practice --report practice.md --json practice.json
.\build\Release\WindowsSecurityWorkbench.exe practice --linux-live --report practice-linux.md --json practice-linux.json
.\build\Release\WindowsSecurityWorkbench.exe full --dry-run --report report.md --json report.json
```

## GUI

GUI-приложение ориентировано на учебное использование и по умолчанию стартует в безопасном режиме, а не в максимально агрессивном полном аудите.

Что уже есть в интерфейсе:

- тёмная тема;
- русский интерфейс;
- быстрые сценарии в левой панели;
- вкладки `Сводка`, `Находки`, `Команды`, `Практика`, `Где искать`, `Обучение`, `JSON`, `Инструменты`;
- экспорт в `Markdown` и `JSON`;
- отдельная опция для Linux live-проверок через `WSL`.

## Docker

Проект является Windows-специфичным: GUI и CLI используют WinAPI и рассчитаны на запуск на Windows.

Поэтому Docker здесь используется не как runtime для GUI, а как воспроизводимый `cross-build`-контейнер, который собирает Windows `.exe` в Linux Docker Desktop через `mingw-w64`.

### Что делает Dockerfile

- поднимает Linux-образ с `cmake`, `ninja` и `mingw-w64`;
- кросс-компилирует `WindowsSecurityWorkbench.exe` и `WindowsSecurityWorkbenchGUI.exe`;
- складывает собранные Windows-бинарники в `/opt/wsw/dist`;
- при `docker run` показывает список готовых артефактов.

### Сборка Docker-образа

```powershell
docker build -t windows-security-workbench:cross .
```

### Запуск контейнера

```powershell
docker run --rm windows-security-workbench:cross
```

Ожидаемый результат: контейнер выведет список собранных `.exe` внутри `/opt/wsw/dist`.

Важно: сами `.exe` нужно запускать уже на Windows-хосте, а не внутри Linux-контейнера.

## Артефакты и экспорт

После запусков приложение может сохранять:

- `report.md` / `report.json`;
- `learn.md` / `learn.json`;
- `study.md` / `study.json`;
- `practice.md` / `practice.json`;
- `practice-linux.md` / `practice-linux.json`;
- локальную историю в каталоге `history/`.

Это удобно и для самообучения, и для приложений к диплому.

## Структура проекта

```text
src/
  app/                 CLI и orchestration сценариев
  core/                общие модели и утилиты
  gui/                 WinAPI GUI
  history/             локальная история запусков
  modules/
    accounts/          аудит учётных записей
    artifacts/         карта системных артефактов
    cleanup/           безопасная очистка
    commands/          каталог Windows/Linux команд
    firewall/          аудит firewall
    lab/               live-запуск read-only команд
    learning/          учебный roadmap
    network/           аудит сети
    playbooks/         практические плейбуки
    services/          аудит служб
    system/            системный аудит Windows
    tasks/             аудит scheduled tasks
    tools/             справочник оснасток и Win+X
    updates/           аудит обновлений
  platform/windows/    Windows-специфичная реализация
  report/              Markdown/JSON экспорт
cmake/toolchains/      toolchain для Docker cross-build
```

## Ограничения

- live-проверки Linux работают только при наличии `WSL`;
- Docker-образ не запускает GUI внутри контейнера, он только собирает Windows-бинарники;
- проект рассчитан в первую очередь на Windows 10/11.

## Безопасностные границы

Проект намеренно ограничен defensive-сценариями. Он:

- не извлекает сохранённые пароли;
- не получает Wi-Fi ключи, токены и иные секреты;
- не делает credential dumping;
- не предназначен для скрытого доступа или offensive-эксплуатации.

## Демонстрация проекта

Удобный сценарий для показа преподавателю:

1. Запустить `.\Start-Workbench.ps1`.
2. Открыть режим практики.
3. Показать категории `Сеть`, `Диски`, `Автозагрузка`, `Журналы`.
4. Выполнить live-проверки Windows.
5. Экспортировать результаты в `Markdown` и `JSON`.
6. Показать, что Linux-команды и сценарии тоже есть в каталоге, а live-режим работает через `WSL`.

## Roadmap

Ближайшие практичные улучшения:

1. Поиск и фильтрация команд по категориям и ключевым словам.
2. Карточки `симптом -> команда -> что смотреть`.
3. Точечный запуск одной команды из GUI.
4. Дифф истории между запусками.
5. SQLite для локальной базы результатов.
6. Скриншоты GUI и branding для GitHub-витрины.
7. Отдельный модуль анализа `Autoruns / Run keys / persistence review`.
