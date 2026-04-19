cmake_minimum_required(VERSION 3.20)

project(WindowsSecurityWorkbench VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(wsw_core STATIC
    src/app/WorkbenchService.cpp
    src/core/AuditTypes.cpp
    src/core/StringUtils.cpp
    src/core/TimeUtils.cpp
    src/history/HistoryStore.cpp
    src/modules/system/SystemAuditModule.cpp
    src/modules/accounts/AccountsAuditModule.cpp
    src/modules/network/NetworkAuditModule.cpp
    src/modules/firewall/FirewallAuditModule.cpp
    src/modules/updates/UpdatesAuditModule.cpp
    src/modules/services/ServicesAuditModule.cpp
    src/modules/tasks/ScheduledTasksAuditModule.cpp
    src/modules/cleanup/CleanupService.cpp
    src/modules/commands/CommandCatalog.cpp
    src/modules/artifacts/ArtifactCatalog.cpp
    src/modules/lab/CommandLabService.cpp
    src/modules/tools/SystemToolCatalog.cpp
    src/modules/learning/LearningCatalog.cpp
    src/modules/playbooks/PlaybookCatalog.cpp
    src/platform/windows/Encoding.cpp
    src/platform/windows/RegistryUtils.cpp
    src/platform/windows/ProcessRunner.cpp
    src/platform/windows/WinError.cpp
    src/report/ReportWriter.cpp
    src/report/JsonWriter.cpp
)

target_include_directories(wsw_core PUBLIC src)

target_compile_definitions(wsw_core PUBLIC
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0A00
    WINVER=0x0A00
)

target_link_libraries(wsw_core PUBLIC
    advapi32
    comctl32
    iphlpapi
    netapi32
    shell32
    ws2_32
)

add_executable(WindowsSecurityWorkbench
    src/main.cpp
    src/app/Application.cpp
)
target_link_libraries(WindowsSecurityWorkbench PRIVATE wsw_core)

add_executable(WindowsSecurityWorkbenchGUI WIN32
    src/gui_main.cpp
    src/gui/WorkbenchWindow.cpp
    src/gui/WorkbenchWindow.h
)
target_link_libraries(WindowsSecurityWorkbenchGUI PRIVATE wsw_core)
target_link_libraries(WindowsSecurityWorkbenchGUI PRIVATE comdlg32)

if(MINGW)
    target_link_options(WindowsSecurityWorkbenchGUI PRIVATE -municode)
endif()
