#pragma once

#include "core/AuditTypes.h"

#include <windows.h>

#include <filesystem>
#include <optional>
#include <string>

namespace sec {

class WorkbenchWindow {
public:
    static int run(HINSTANCE instance, int showCommand);

private:
    enum class Action {
        audit,
        full,
        cleanup,
        learn,
        practice,
        study,
    };

    struct JobOptions {
        Action action {Action::full};
        bool dryRun {true};
        bool includeCommands {true};
        bool includeStudy {true};
        bool includeLinuxLive {false};
    };

    struct JobResult {
        bool success {false};
        ExecutionBundle bundle;
        std::wstring errorText;
        std::optional<std::filesystem::path> historyPath;
    };

    explicit WorkbenchWindow(HINSTANCE instance);
    ~WorkbenchWindow();

    bool create(int showCommand);
    void show(int showCommand) const;

    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void createFonts();
    void destroyFonts();
    void createControls();
    void layoutControls(int clientWidth, int clientHeight) const;

    void onCommand(WORD controlId);
    void onTabChanged();
    void onJobCompleted(JobResult* result);

    void startAction(Action action);
    void setBusyState(bool busy, const std::wstring& statusText);
    JobOptions readJobOptions(Action action) const;

    void updateViewText();
    void updateViewerFont();
    std::wstring actionDisplayName(Action action) const;
    std::wstring currentTabText() const;
    std::wstring buildOverviewText() const;
    std::wstring buildFindingsText() const;
    std::wstring buildCommandsText() const;
    std::wstring buildPracticeText() const;
    std::wstring buildToolsText() const;
    std::wstring buildLocationsText() const;
    std::wstring buildStudyText() const;
    std::wstring buildJsonText() const;

    void saveMarkdown();
    void saveJson();
    std::optional<std::filesystem::path> promptSavePath(const wchar_t* title,
                                                        const wchar_t* filter,
                                                        const wchar_t* defaultExtension,
                                                        const std::wstring& defaultName) const;

    HINSTANCE instance_ {nullptr};
    HWND hwnd_ {nullptr};
    HWND titleLabel_ {nullptr};
    HWND subtitleLabel_ {nullptr};
    HWND sidebarTitle_ {nullptr};
    HWND auditButton_ {nullptr};
    HWND fullButton_ {nullptr};
    HWND cleanupButton_ {nullptr};
    HWND learnButton_ {nullptr};
    HWND practiceButton_ {nullptr};
    HWND studyButton_ {nullptr};
    HWND optionsTitle_ {nullptr};
    HWND dryRunCheck_ {nullptr};
    HWND includeCommandsCheck_ {nullptr};
    HWND includeStudyCheck_ {nullptr};
    HWND includeLinuxLiveCheck_ {nullptr};
    HWND exportTitle_ {nullptr};
    HWND saveMarkdownButton_ {nullptr};
    HWND saveJsonButton_ {nullptr};
    HWND tabControl_ {nullptr};
    HWND viewer_ {nullptr};
    HWND statusLabel_ {nullptr};

    HFONT titleFont_ {nullptr};
    HFONT subtitleFont_ {nullptr};
    HFONT bodyFont_ {nullptr};
    HFONT monoFont_ {nullptr};
    HBRUSH backgroundBrush_ {nullptr};

    bool busy_ {false};
    ExecutionBundle currentBundle_;
    std::optional<std::filesystem::path> lastHistoryPath_;
};

}  // namespace sec
