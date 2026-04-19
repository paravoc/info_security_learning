#include "gui/WorkbenchWindow.h"

#include "app/WorkbenchService.h"
#include "core/AuditTypes.h"
#include "core/StringUtils.h"
#include "core/TimeUtils.h"
#include "history/HistoryStore.h"
#include "platform/windows/Encoding.h"
#include "report/JsonWriter.h"
#include "report/ReportWriter.h"

#include <richedit.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>

#include <algorithm>
#include <array>
#include <memory>
#include <sstream>
#include <thread>

namespace {

constexpr UINT kMessageJobCompleted = WM_APP + 1;

constexpr int kIdAudit = 1001;
constexpr int kIdFull = 1002;
constexpr int kIdCleanup = 1003;
constexpr int kIdLearn = 1004;
constexpr int kIdPractice = 1005;
constexpr int kIdStudy = 1006;
constexpr int kIdDryRun = 1101;
constexpr int kIdIncludeCommands = 1102;
constexpr int kIdIncludeStudy = 1103;
constexpr int kIdIncludeLinuxLive = 1104;
constexpr int kIdSaveMarkdown = 1201;
constexpr int kIdSaveJson = 1202;
constexpr int kIdTabs = 1301;
constexpr int kIdViewer = 1302;

constexpr COLORREF kColorBackground = RGB(12, 16, 22);
constexpr COLORREF kColorSidebar = RGB(18, 23, 31);
constexpr COLORREF kColorContent = RGB(20, 25, 34);
constexpr COLORREF kColorViewer = RGB(15, 19, 27);
constexpr COLORREF kColorBorder = RGB(43, 51, 66);
constexpr COLORREF kColorBorderSoft = RGB(31, 38, 50);
constexpr COLORREF kColorTextPrimary = RGB(236, 240, 246);
constexpr COLORREF kColorTextMuted = RGB(152, 163, 181);
constexpr COLORREF kColorAccent = RGB(74, 115, 255);
constexpr COLORREF kColorAccentSoft = RGB(37, 53, 93);
constexpr COLORREF kColorButton = RGB(28, 35, 47);
constexpr COLORREF kColorButtonPressed = RGB(22, 28, 38);
constexpr COLORREF kColorButtonDisabled = RGB(24, 28, 36);

std::wstring joinLines(const std::vector<std::string>& lines, const std::wstring& bulletPrefix) {
    std::wstring output;
    for (const auto& line : lines) {
        output += bulletPrefix;
        output += sec::toWide(line);
        output += L"\r\n";
    }
    return output;
}

std::wstring scenarioDisplayName(const std::string& scenario) {
    if (scenario == "audit") {
        return L"Быстрый аудит";
    }
    if (scenario == "cleanup") {
        return L"Очистка и кэш";
    }
    if (scenario == "learn") {
        return L"Команды и шпаргалки";
    }
    if (scenario == "practice") {
        return L"Практика и live-проверки";
    }
    if (scenario == "study") {
        return L"План обучения";
    }
    if (scenario == "full") {
        return L"Полная проверка";
    }
    return sec::toWide(scenario);
}

HMENU menuId(const int value) {
    return reinterpret_cast<HMENU>(static_cast<INT_PTR>(value));
}

std::wstring getWindowTextCopy(const HWND hwnd) {
    const auto length = GetWindowTextLengthW(hwnd);
    std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
    GetWindowTextW(hwnd, text.data(), length + 1);
    text.resize(static_cast<std::size_t>(length));
    return text;
}

bool isActionButtonId(const int id) {
    return id == kIdAudit || id == kIdFull || id == kIdCleanup || id == kIdLearn || id == kIdPractice || id == kIdStudy;
}

bool isExportButtonId(const int id) {
    return id == kIdSaveMarkdown || id == kIdSaveJson;
}

RECT insetRect(RECT rect, const int dx, const int dy) {
    rect.left += dx;
    rect.top += dy;
    rect.right -= dx;
    rect.bottom -= dy;
    return rect;
}

void fillRoundedRect(const HDC deviceContext,
                     const RECT& rect,
                     const COLORREF fillColor,
                     const COLORREF borderColor,
                     const int radius = 16) {
    const auto brush = CreateSolidBrush(fillColor);
    const auto pen = CreatePen(PS_SOLID, 1, borderColor);
    const auto oldBrush = SelectObject(deviceContext, brush);
    const auto oldPen = SelectObject(deviceContext, pen);
    RoundRect(deviceContext, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(deviceContext, oldBrush);
    SelectObject(deviceContext, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void fillRectColor(const HDC deviceContext, const RECT& rect, const COLORREF color) {
    const auto brush = CreateSolidBrush(color);
    FillRect(deviceContext, &rect, brush);
    DeleteObject(brush);
}

void drawCenteredText(const HDC deviceContext,
                      RECT rect,
                      const std::wstring& text,
                      const COLORREF color,
                      const UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE) {
    SetBkMode(deviceContext, TRANSPARENT);
    SetTextColor(deviceContext, color);
    DrawTextW(deviceContext, text.c_str(), -1, &rect, format);
}

void applyViewerTheme(const HWND viewer) {
    SendMessageW(viewer, EM_SETBKGNDCOLOR, 0, kColorViewer);

    CHARFORMAT2W format {};
    format.cbSize = sizeof(format);
    format.dwMask = CFM_COLOR;
    format.crTextColor = kColorTextPrimary;
    SendMessageW(viewer, EM_SETCHARFORMAT, SCF_DEFAULT, reinterpret_cast<LPARAM>(&format));
    SendMessageW(viewer, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&format));
}

void paintButton(const DRAWITEMSTRUCT& drawInfo) {
    const int controlId = static_cast<int>(drawInfo.CtlID);
    const bool isPressed = (drawInfo.itemState & ODS_SELECTED) != 0;
    const bool isDisabled = (drawInfo.itemState & ODS_DISABLED) != 0;

    COLORREF fillColor = kColorButton;
    COLORREF borderColor = kColorBorder;
    COLORREF textColor = kColorTextPrimary;

    if (isDisabled) {
        fillColor = kColorButtonDisabled;
        borderColor = kColorBorderSoft;
        textColor = kColorTextMuted;
    } else if (controlId == kIdFull) {
        fillColor = isPressed ? RGB(58, 96, 225) : kColorAccent;
        borderColor = RGB(100, 138, 255);
    } else if (isActionButtonId(controlId)) {
        fillColor = isPressed ? kColorButtonPressed : kColorButton;
        borderColor = RGB(69, 82, 104);
    } else if (isExportButtonId(controlId)) {
        fillColor = isPressed ? RGB(26, 38, 68) : kColorAccentSoft;
        borderColor = RGB(82, 109, 170);
    }

    auto rect = insetRect(drawInfo.rcItem, 2, 2);
    fillRoundedRect(drawInfo.hDC, rect, fillColor, borderColor, 18);

    if ((drawInfo.itemState & ODS_FOCUS) != 0) {
        auto focusRect = insetRect(rect, 4, 4);
        DrawFocusRect(drawInfo.hDC, &focusRect);
    }

    drawCenteredText(drawInfo.hDC, rect, getWindowTextCopy(drawInfo.hwndItem), textColor);
}

LRESULT drawTabItem(const LPNMCUSTOMDRAW customDraw, const HWND tabControl) {
    if (customDraw->dwDrawStage == CDDS_PREPAINT) {
        fillRectColor(customDraw->hdc, customDraw->rc, kColorContent);
        return CDRF_NOTIFYITEMDRAW;
    }

    if (customDraw->dwDrawStage != CDDS_ITEMPREPAINT) {
        return CDRF_DODEFAULT;
    }

    RECT itemRect {};
    const auto itemIndex = static_cast<int>(customDraw->dwItemSpec);
    TabCtrl_GetItemRect(tabControl, itemIndex, &itemRect);

    wchar_t buffer[64] {};
    TCITEMW item {};
    item.mask = TCIF_TEXT;
    item.pszText = buffer;
    item.cchTextMax = static_cast<int>(std::size(buffer));
    TabCtrl_GetItem(tabControl, itemIndex, &item);

    const bool selected = TabCtrl_GetCurSel(tabControl) == itemIndex;
    auto fillRect = insetRect(itemRect, 4, 4);
    fillRect.bottom -= 2;

    fillRoundedRect(customDraw->hdc,
                    fillRect,
                    selected ? kColorAccentSoft : kColorContent,
                    selected ? RGB(86, 118, 224) : kColorBorderSoft,
                    14);

    if (selected) {
        RECT accentRect = fillRect;
        accentRect.top = accentRect.bottom - 4;
        fillRectColor(customDraw->hdc, accentRect, kColorAccent);
    }

    drawCenteredText(customDraw->hdc, fillRect, buffer, selected ? kColorTextPrimary : kColorTextMuted);
    return CDRF_SKIPDEFAULT;
}

void paintPanels(const HWND hwnd, const HDC deviceContext) {
    RECT clientRect {};
    GetClientRect(hwnd, &clientRect);
    fillRectColor(deviceContext, clientRect, kColorBackground);

    const int width = std::max(clientRect.right - clientRect.left, 980L);
    const int height = std::max(clientRect.bottom - clientRect.top, 760L);
    const int margin = 24;
    const int headerHeight = 104;
    const int statusHeight = 30;
    const int sidebarWidth = 320;
    const int contentX = margin + sidebarWidth + margin;
    const int contentTop = headerHeight + margin;

    RECT sidebarRect {margin - 4, contentTop - 12, margin + sidebarWidth + 4, height - statusHeight - 18};
    RECT contentRect {contentX - 8, contentTop - 12, width - margin + 4, height - statusHeight - 18};
    RECT headerLine {margin, headerHeight + 4, width - margin, headerHeight + 5};

    fillRoundedRect(deviceContext, sidebarRect, kColorSidebar, kColorBorderSoft, 24);
    fillRoundedRect(deviceContext, contentRect, kColorContent, kColorBorderSoft, 24);
    fillRectColor(deviceContext, headerLine, kColorBorderSoft);
}

}  // namespace

namespace sec {

WorkbenchWindow::WorkbenchWindow(const HINSTANCE instance) : instance_(instance) {}

WorkbenchWindow::~WorkbenchWindow() {
    destroyFonts();
    if (backgroundBrush_ != nullptr) {
        DeleteObject(backgroundBrush_);
    }
}

int WorkbenchWindow::run(const HINSTANCE instance, const int showCommand) {
    LoadLibraryW(L"Msftedit.dll");

    INITCOMMONCONTROLSEX controls {};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

    WorkbenchWindow window(instance);
    if (!window.create(showCommand)) {
        return 1;
    }

    window.show(showCommand);

    MSG message {};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

bool WorkbenchWindow::create(const int showCommand) {
    createFonts();
    backgroundBrush_ = CreateSolidBrush(kColorBackground);

    WNDCLASSW windowClass {};
    windowClass.lpfnWndProc = &WorkbenchWindow::windowProc;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = backgroundBrush_;
    windowClass.lpszClassName = L"WindowsSecurityWorkbenchGuiWindow";
    windowClass.style = CS_HREDRAW | CS_VREDRAW;

    if (RegisterClassW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    hwnd_ = CreateWindowExW(0,
                            windowClass.lpszClassName,
                            L"Windows Security Workbench",
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            1520,
                            940,
                            nullptr,
                            nullptr,
                            instance_,
                            this);

    if (hwnd_ == nullptr) {
        return false;
    }

    createControls();
    layoutControls(1520, 940);
    startAction(Action::learn);
    return true;
}

void WorkbenchWindow::show(const int showCommand) const {
    ShowWindow(hwnd_, showCommand);
    UpdateWindow(hwnd_);
}

LRESULT CALLBACK WorkbenchWindow::windowProc(const HWND hwnd, const UINT message, const WPARAM wParam, const LPARAM lParam) {
    WorkbenchWindow* window = nullptr;
    if (message == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = static_cast<WorkbenchWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->hwnd_ = hwnd;
    } else {
        window = reinterpret_cast<WorkbenchWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (window != nullptr) {
        return window->handleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT WorkbenchWindow::handleMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) {
    switch (message) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT paintStruct {};
            const auto deviceContext = BeginPaint(hwnd_, &paintStruct);
            paintPanels(hwnd_, deviceContext);
            EndPaint(hwnd_, &paintStruct);
            return 0;
        }
        case WM_SIZE:
            layoutControls(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            InvalidateRect(hwnd_, nullptr, TRUE);
            return 0;
        case WM_COMMAND:
            onCommand(LOWORD(wParam));
            return 0;
        case WM_DRAWITEM: {
            auto* drawInfo = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
            if (drawInfo != nullptr && drawInfo->CtlType == ODT_BUTTON) {
                paintButton(*drawInfo);
                return TRUE;
            }
            return FALSE;
        }
        case WM_NOTIFY: {
            auto* header = reinterpret_cast<LPNMHDR>(lParam);
            if (header->idFrom == kIdTabs && header->code == TCN_SELCHANGE) {
                onTabChanged();
                return 0;
            }
            if (header->idFrom == kIdTabs && header->code == NM_CUSTOMDRAW) {
                return drawTabItem(reinterpret_cast<LPNMCUSTOMDRAW>(lParam), tabControl_);
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            auto deviceContext = reinterpret_cast<HDC>(wParam);
            SetBkMode(deviceContext, TRANSPARENT);
            SetTextColor(deviceContext, reinterpret_cast<HWND>(lParam) == subtitleLabel_ ? kColorTextMuted : kColorTextPrimary);
            return reinterpret_cast<INT_PTR>(backgroundBrush_);
        }
        case WM_CTLCOLORBTN: {
            auto deviceContext = reinterpret_cast<HDC>(wParam);
            SetBkColor(deviceContext, kColorSidebar);
            SetTextColor(deviceContext, kColorTextPrimary);
            return reinterpret_cast<INT_PTR>(backgroundBrush_);
        }
        case kMessageJobCompleted:
            onJobCompleted(reinterpret_cast<JobResult*>(lParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd_, message, wParam, lParam);
    }
}

void WorkbenchWindow::createFonts() {
    titleFont_ = CreateFontW(34, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    subtitleFont_ = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    bodyFont_ = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    monoFont_ = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
}

void WorkbenchWindow::destroyFonts() {
    if (titleFont_ != nullptr) {
        DeleteObject(titleFont_);
    }
    if (subtitleFont_ != nullptr) {
        DeleteObject(subtitleFont_);
    }
    if (bodyFont_ != nullptr) {
        DeleteObject(bodyFont_);
    }
    if (monoFont_ != nullptr) {
        DeleteObject(monoFont_);
    }
}

void WorkbenchWindow::createControls() {
    const DWORD labelStyle = WS_CHILD | WS_VISIBLE;
    const DWORD buttonStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
    const DWORD checkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;

    titleLabel_ = CreateWindowW(L"STATIC", L"Windows Security Workbench", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);
    subtitleLabel_ = CreateWindowW(L"STATIC", L"Учебный тренажёр по Windows/Linux: команды, практические сценарии, live-проверки и карта системных артефактов.", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);
    sidebarTitle_ = CreateWindowW(L"STATIC", L"Сценарии", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);
    optionsTitle_ = CreateWindowW(L"STATIC", L"Параметры", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);
    exportTitle_ = CreateWindowW(L"STATIC", L"Экспорт", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);

    auditButton_ = CreateWindowW(L"BUTTON", L"Быстрый аудит", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdAudit), instance_, nullptr);
    fullButton_ = CreateWindowW(L"BUTTON", L"Полная проверка", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdFull), instance_, nullptr);
    cleanupButton_ = CreateWindowW(L"BUTTON", L"Очистка и кэш", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdCleanup), instance_, nullptr);
    learnButton_ = CreateWindowW(L"BUTTON", L"Команды и шпаргалки", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdLearn), instance_, nullptr);
    practiceButton_ = CreateWindowW(L"BUTTON", L"Практика и тесты", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdPractice), instance_, nullptr);
    studyButton_ = CreateWindowW(L"BUTTON", L"План обучения", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdStudy), instance_, nullptr);

    dryRunCheck_ = CreateWindowW(L"BUTTON", L"Только dry-run для очистки", checkStyle, 0, 0, 0, 0, hwnd_, menuId(kIdDryRun), instance_, nullptr);
    includeCommandsCheck_ = CreateWindowW(L"BUTTON", L"Добавить каталог команд", checkStyle, 0, 0, 0, 0, hwnd_, menuId(kIdIncludeCommands), instance_, nullptr);
    includeStudyCheck_ = CreateWindowW(L"BUTTON", L"Добавить roadmap и справку", checkStyle, 0, 0, 0, 0, hwnd_, menuId(kIdIncludeStudy), instance_, nullptr);
    includeLinuxLiveCheck_ = CreateWindowW(L"BUTTON", L"Пробовать Linux через WSL", checkStyle, 0, 0, 0, 0, hwnd_, menuId(kIdIncludeLinuxLive), instance_, nullptr);

    saveMarkdownButton_ = CreateWindowW(L"BUTTON", L"Сохранить Markdown", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdSaveMarkdown), instance_, nullptr);
    saveJsonButton_ = CreateWindowW(L"BUTTON", L"Сохранить JSON", buttonStyle, 0, 0, 0, 0, hwnd_, menuId(kIdSaveJson), instance_, nullptr);

    tabControl_ = CreateWindowW(WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP, 0, 0, 0, 0, hwnd_, menuId(kIdTabs), instance_, nullptr);
    viewer_ = CreateWindowExW(WS_EX_CLIENTEDGE,
                              MSFTEDIT_CLASS,
                              L"",
                              WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | WS_VSCROLL | WS_HSCROLL,
                              0,
                              0,
                              0,
                              0,
                              hwnd_,
                              menuId(kIdViewer),
                              instance_,
                              nullptr);

    statusLabel_ = CreateWindowW(L"STATIC", L"Готово к запуску.", labelStyle, 0, 0, 0, 0, hwnd_, nullptr, instance_, nullptr);

    SendMessageW(dryRunCheck_, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(includeCommandsCheck_, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(includeStudyCheck_, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(viewer_, EM_SETLIMITTEXT, 4 * 1024 * 1024, 0);
    applyViewerTheme(viewer_);

    TCITEMW item {};
    item.mask = TCIF_TEXT;
    item.pszText = const_cast<LPWSTR>(L"Сводка");
    TabCtrl_InsertItem(tabControl_, 0, &item);
    item.pszText = const_cast<LPWSTR>(L"Находки");
    TabCtrl_InsertItem(tabControl_, 1, &item);
    item.pszText = const_cast<LPWSTR>(L"Команды");
    TabCtrl_InsertItem(tabControl_, 2, &item);
    item.pszText = const_cast<LPWSTR>(L"Практика");
    TabCtrl_InsertItem(tabControl_, 3, &item);
    item.pszText = const_cast<LPWSTR>(L"Где искать");
    TabCtrl_InsertItem(tabControl_, 4, &item);
    item.pszText = const_cast<LPWSTR>(L"Обучение");
    TabCtrl_InsertItem(tabControl_, 5, &item);
    item.pszText = const_cast<LPWSTR>(L"JSON");
    TabCtrl_InsertItem(tabControl_, 6, &item);
    item.pszText = const_cast<LPWSTR>(L"Инструменты");
    TabCtrl_InsertItem(tabControl_, 7, &item);

    for (const auto control : {titleLabel_, subtitleLabel_, sidebarTitle_, optionsTitle_, exportTitle_, auditButton_, fullButton_, cleanupButton_, learnButton_, practiceButton_, studyButton_, dryRunCheck_, includeCommandsCheck_, includeStudyCheck_, includeLinuxLiveCheck_, saveMarkdownButton_, saveJsonButton_, tabControl_, statusLabel_}) {
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(bodyFont_), TRUE);
    }
    SendMessageW(titleLabel_, WM_SETFONT, reinterpret_cast<WPARAM>(titleFont_), TRUE);
    SendMessageW(subtitleLabel_, WM_SETFONT, reinterpret_cast<WPARAM>(subtitleFont_), TRUE);
    SendMessageW(viewer_, WM_SETFONT, reinterpret_cast<WPARAM>(bodyFont_), TRUE);
}

void WorkbenchWindow::layoutControls(const int clientWidth, const int clientHeight) const {
    const int width = std::max(clientWidth, 980);
    const int height = std::max(clientHeight, 760);
    const int margin = 24;
    const int headerHeight = 104;
    const int statusHeight = 30;
    const int sidebarWidth = 320;
    const int contentX = margin + sidebarWidth + margin;
    const int contentWidth = width - contentX - margin;
    const int contentTop = headerHeight + margin;
    const int contentHeight = height - contentTop - statusHeight - margin;

    MoveWindow(titleLabel_, margin, 20, width - (2 * margin), 40, TRUE);
    MoveWindow(subtitleLabel_, margin, 60, width - (2 * margin), 28, TRUE);

    MoveWindow(sidebarTitle_, margin, contentTop, sidebarWidth, 24, TRUE);
    MoveWindow(auditButton_, margin, contentTop + 36, sidebarWidth, 40, TRUE);
    MoveWindow(fullButton_, margin, contentTop + 82, sidebarWidth, 40, TRUE);
    MoveWindow(cleanupButton_, margin, contentTop + 128, sidebarWidth, 40, TRUE);
    MoveWindow(learnButton_, margin, contentTop + 174, sidebarWidth, 40, TRUE);
    MoveWindow(practiceButton_, margin, contentTop + 220, sidebarWidth, 40, TRUE);
    MoveWindow(studyButton_, margin, contentTop + 266, sidebarWidth, 40, TRUE);

    MoveWindow(optionsTitle_, margin, contentTop + 320, sidebarWidth, 24, TRUE);
    MoveWindow(dryRunCheck_, margin, contentTop + 352, sidebarWidth, 24, TRUE);
    MoveWindow(includeCommandsCheck_, margin, contentTop + 380, sidebarWidth, 24, TRUE);
    MoveWindow(includeStudyCheck_, margin, contentTop + 408, sidebarWidth, 24, TRUE);
    MoveWindow(includeLinuxLiveCheck_, margin, contentTop + 436, sidebarWidth, 24, TRUE);

    MoveWindow(exportTitle_, margin, contentTop + 482, sidebarWidth, 24, TRUE);
    MoveWindow(saveMarkdownButton_, margin, contentTop + 516, sidebarWidth, 38, TRUE);
    MoveWindow(saveJsonButton_, margin, contentTop + 560, sidebarWidth, 38, TRUE);

    MoveWindow(tabControl_, contentX, contentTop, contentWidth, 42, TRUE);
    MoveWindow(viewer_, contentX, contentTop + 46, contentWidth, contentHeight - 46, TRUE);
    MoveWindow(statusLabel_, margin, height - statusHeight - 8, width - (2 * margin), statusHeight, TRUE);
}

void WorkbenchWindow::onCommand(const WORD controlId) {
    switch (controlId) {
        case kIdAudit:
            startAction(Action::audit);
            break;
        case kIdFull:
            startAction(Action::full);
            break;
        case kIdCleanup:
            startAction(Action::cleanup);
            break;
        case kIdLearn:
            startAction(Action::learn);
            break;
        case kIdPractice:
            startAction(Action::practice);
            break;
        case kIdStudy:
            startAction(Action::study);
            break;
        case kIdSaveMarkdown:
            saveMarkdown();
            break;
        case kIdSaveJson:
            saveJson();
            break;
        default:
            break;
    }
}

void WorkbenchWindow::onTabChanged() {
    updateViewText();
}

void WorkbenchWindow::onJobCompleted(JobResult* result) {
    std::unique_ptr<JobResult> holder(result);
    if (!holder->success) {
        setBusyState(false, L"Выполнение завершилось с ошибкой.");
        MessageBoxW(hwnd_, holder->errorText.c_str(), L"Windows Security Workbench", MB_ICONERROR | MB_OK);
        return;
    }

    currentBundle_ = std::move(holder->bundle);
    lastHistoryPath_ = holder->historyPath;
    if (currentBundle_.scenario == "learn") {
        TabCtrl_SetCurSel(tabControl_, 2);
    } else if (currentBundle_.scenario == "practice") {
        TabCtrl_SetCurSel(tabControl_, 3);
    } else if (currentBundle_.scenario == "study") {
        TabCtrl_SetCurSel(tabControl_, 5);
    } else {
        TabCtrl_SetCurSel(tabControl_, 0);
    }
    updateViewText();

    std::wstring status = L"Готово: " + scenarioDisplayName(currentBundle_.scenario);
    if (lastHistoryPath_.has_value()) {
        status += L" | history: " + lastHistoryPath_->wstring();
    }
    setBusyState(false, status);
}

void WorkbenchWindow::startAction(const Action action) {
    if (busy_) {
        return;
    }

    const auto options = readJobOptions(action);
    setBusyState(true, L"Выполняю: " + actionDisplayName(action) + L"...");

    const auto targetWindow = hwnd_;
    std::thread([options, targetWindow]() {
        auto* result = new JobResult();

        try {
            const WorkbenchService service;
            switch (options.action) {
                case Action::audit:
                    result->bundle = service.buildAuditBundle(options.includeCommands, options.includeStudy);
                    break;
                case Action::full:
                    result->bundle = service.buildFullBundle(options.dryRun);
                    break;
                case Action::cleanup:
                    result->bundle = service.buildCleanupBundle(options.dryRun);
                    break;
                case Action::learn:
                    result->bundle = service.buildLearnBundle();
                    break;
                case Action::practice:
                    result->bundle = service.buildPracticeBundle(options.includeLinuxLive);
                    break;
                case Action::study:
                    result->bundle = service.buildStudyBundle();
                    break;
            }

            result->historyPath = HistoryStore {}.save(result->bundle);
            result->success = true;
        } catch (const std::exception& error) {
            result->success = false;
            result->errorText = toWide(error.what());
        }

        if (!PostMessageW(targetWindow, kMessageJobCompleted, 0, reinterpret_cast<LPARAM>(result))) {
            delete result;
        }
    }).detach();
}

void WorkbenchWindow::setBusyState(const bool busy, const std::wstring& statusText) {
    busy_ = busy;
    for (const auto control : {auditButton_, fullButton_, cleanupButton_, learnButton_, practiceButton_, studyButton_, saveMarkdownButton_, saveJsonButton_}) {
        EnableWindow(control, !busy);
        InvalidateRect(control, nullptr, TRUE);
    }

    SetWindowTextW(statusLabel_, statusText.c_str());
}

WorkbenchWindow::JobOptions WorkbenchWindow::readJobOptions(const Action action) const {
    JobOptions options;
    options.action = action;
    options.dryRun = SendMessageW(dryRunCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    options.includeCommands = SendMessageW(includeCommandsCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    options.includeStudy = SendMessageW(includeStudyCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    options.includeLinuxLive = SendMessageW(includeLinuxLiveCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    return options;
}

void WorkbenchWindow::updateViewText() {
    updateViewerFont();
    const auto text = currentTabText();
    SetWindowTextW(viewer_, text.c_str());
    applyViewerTheme(viewer_);
}

void WorkbenchWindow::updateViewerFont() {
    const auto currentTab = TabCtrl_GetCurSel(tabControl_);
    const auto font = (currentTab == 2 || currentTab == 3 || currentTab == 6 || currentTab == 7) ? monoFont_ : bodyFont_;
    SendMessageW(viewer_, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

std::wstring WorkbenchWindow::actionDisplayName(const Action action) const {
    switch (action) {
        case Action::audit:
            return L"Быстрый аудит";
        case Action::full:
            return L"Полная проверка";
        case Action::cleanup:
            return L"Очистка и кэш";
        case Action::learn:
            return L"Команды и шпаргалки";
        case Action::practice:
            return L"Практика и тесты";
        case Action::study:
            return L"План обучения";
    }

    return L"Сценарий";
}

std::wstring WorkbenchWindow::currentTabText() const {
    switch (TabCtrl_GetCurSel(tabControl_)) {
        case 0:
            return buildOverviewText();
        case 1:
            return buildFindingsText();
        case 2:
            return buildCommandsText();
        case 3:
            return buildPracticeText();
        case 4:
            return buildLocationsText();
        case 5:
            return buildStudyText();
        case 6:
            return buildJsonText();
        case 7:
            return buildToolsText();
        default:
            return buildOverviewText();
    }
}

std::wstring WorkbenchWindow::buildOverviewText() const {
    std::wostringstream stream;
    stream << L"Сценарий: " << scenarioDisplayName(currentBundle_.scenario) << L"\r\n";
    stream << L"Сформировано: " << toWide(currentTimestampHuman()) << L"\r\n";
    if (lastHistoryPath_.has_value()) {
        stream << L"Снимок истории: " << lastHistoryPath_->wstring() << L"\r\n";
    }
    stream << L"\r\n";

    int okCount = 0;
    int warningCount = 0;
    int errorCount = 0;
    for (const auto& item : currentBundle_.auditItems) {
        switch (item.status) {
            case AuditStatus::ok:
                ++okCount;
                break;
            case AuditStatus::warning:
                ++warningCount;
                break;
            case AuditStatus::error:
                ++errorCount;
                break;
            default:
                break;
        }
    }

    stream << L"Содержимое набора\r\n";
    stream << L"  Аудит: " << currentBundle_.auditItems.size() << L" пунктов\r\n";
    if (currentBundle_.cleanupSummary.has_value()) {
        stream << L"  Очистка: " << currentBundle_.cleanupSummary->targets.size() << L" целей\r\n";
    } else {
        stream << L"  Очистка: не запускалась\r\n";
    }
    stream << L"  Команды: " << currentBundle_.commands.size() << L"\r\n";
    stream << L"  Практические сценарии: " << currentBundle_.playbooks.size() << L"\r\n";
    stream << L"  Live-проверки: " << currentBundle_.liveCommandResults.size() << L"\r\n";
    stream << L"  Инструменты: " << currentBundle_.toolReferences.size() << L"\r\n";
    stream << L"  Где искать: " << currentBundle_.artifactLocations.size() << L"\r\n";
    stream << L"  Учебные блоки: " << currentBundle_.learningSections.size() << L"\r\n\r\n";

    stream << L"Статус\r\n";
    stream << L"  Норма: " << okCount << L"\r\n";
    stream << L"  Риски: " << warningCount << L"\r\n";
    stream << L"  Ошибки: " << errorCount << L"\r\n\r\n";

    if (currentBundle_.cleanupSummary.has_value()) {
        stream << L"Режим очистки: " << (currentBundle_.cleanupSummary->dryRun ? L"dry-run" : L"применение") << L"\r\n\r\n";
    }

    if (!currentBundle_.auditItems.empty()) {
        stream << L"Главные наблюдения\r\n";
        std::size_t shown = 0;
        for (const auto& item : currentBundle_.auditItems) {
            if (item.status != AuditStatus::warning && item.status != AuditStatus::error) {
                continue;
            }
            stream << L"  - [" << toWide(toDisplayString(item.status)) << L"] " << toWide(item.category) << L" :: " << toWide(item.title) << L"\r\n";
            ++shown;
            if (shown == 8) {
                break;
            }
        }
        if (shown == 0) {
            stream << L"  - В текущем наборе нет предупреждений и ошибок.\r\n";
        }
    } else {
        stream << L"В текущем наборе нет результатов аудита.\r\n";
    }

    return stream.str();
}

std::wstring WorkbenchWindow::buildFindingsText() const {
    std::wstring text;

    for (const auto& item : currentBundle_.auditItems) {
        text += L"[";
        text += toWide(toDisplayString(item.status));
        text += L"] ";
        text += toWide(item.category);
        text += L" :: ";
        text += toWide(item.title);
        text += L"\r\n";
        text += joinLines(item.details, L"  - ");
        if (!item.recommendation.empty()) {
            text += L"  Рекомендация: ";
            text += toWide(item.recommendation);
            text += L"\r\n";
        }
        text += L"\r\n";
    }

    if (currentBundle_.cleanupSummary.has_value()) {
        text += L"Очистка и кэш\r\n";
        text += L"Режим: ";
        text += (currentBundle_.cleanupSummary->dryRun ? L"dry-run" : L"применение");
        text += L"\r\n\r\n";
        for (const auto& target : currentBundle_.cleanupSummary->targets) {
            text += L"- ";
            text += toWide(target.target);
            text += L" | files=";
            text += std::to_wstring(target.filesAffected);
            text += L" | bytes=";
            text += toWide(humanReadableBytes(target.bytesAffected));
            text += L" | note=";
            text += toWide(target.message);
            text += L"\r\n";
        }
    }

    if (text.empty()) {
        text = L"Пока нет результатов. Запусти один из сценариев слева.";
    }
    return text;
}

std::wstring WorkbenchWindow::buildCommandsText() const {
    std::wstring text;
    std::string currentPlatform;
    std::string currentCategory;
    for (const auto& command : currentBundle_.commands) {
        if (command.platform != currentPlatform || command.category != currentCategory) {
            currentPlatform = command.platform;
            currentCategory = command.category;
            if (!text.empty()) {
                text += L"\r\n";
            }
            text += L"=== ";
            text += toWide(currentPlatform);
            text += L" :: ";
            text += toWide(currentCategory);
            text += L" ===\r\n\r\n";
        }

        text += toWide(command.title);
        text += L"\r\n";
        if (!command.relatedTool.empty()) {
            text += L"  Связано с GUI: ";
            text += toWide(command.relatedTool);
            text += L"\r\n";
        }
        if (!command.scenario.empty()) {
            text += L"  Когда использовать: ";
            text += toWide(command.scenario);
            text += L"\r\n";
        }
        text += L"  Команда      : ";
        text += toWide(command.command);
        text += L"\r\n";
        text += L"  Назначение   : ";
        text += toWide(command.purpose);
        text += L"\r\n";
        text += L"  Осторожно    : ";
        text += toWide(command.caution);
        text += L"\r\n";
        text += L"  Практика здесь: ";
        text += (command.runnableInsideApp ? L"да" : L"нет");
        text += L"\r\n";
        if (!command.exampleOutput.empty()) {
            text += L"  Что получится:\r\n";
            text += L"    ------------------------------\r\n";
            text += joinLines(splitLines(command.exampleOutput), L"    ");
            text += L"    ------------------------------\r\n";
        }
        if (!command.interpretation.empty()) {
            text += L"  Как читать   : ";
            text += toWide(command.interpretation);
            text += L"\r\n";
        }
        if (!command.checks.empty()) {
            text += L"  На что смотреть:\r\n";
            text += joinLines(command.checks, L"    - ");
        }
        if (!command.nextCommands.empty()) {
            text += L"  Что проверить потом:\r\n";
            text += joinLines(command.nextCommands, L"    - ");
        }
        text += L"\r\n";
    }

    if (text.empty()) {
        text = L"В текущем наборе нет каталога команд.";
    }
    return text;
}

std::wstring WorkbenchWindow::buildPracticeText() const {
    std::wstring text;
    std::string currentPlatform;
    std::string currentCategory;

    if (!currentBundle_.playbooks.empty()) {
        text += L"=== Практические сценарии ===\r\n\r\n";
        for (const auto& playbook : currentBundle_.playbooks) {
            if (playbook.platform != currentPlatform || playbook.category != currentCategory) {
                currentPlatform = playbook.platform;
                currentCategory = playbook.category;
                text += L"--- ";
                text += toWide(currentPlatform);
                text += L" :: ";
                text += toWide(currentCategory);
                text += L" ---\r\n\r\n";
            }

            text += toWide(playbook.title);
            text += L"\r\n";
            text += L"  Симптом      : ";
            text += toWide(playbook.symptom);
            text += L"\r\n";
            text += L"  Цель         : ";
            text += toWide(playbook.goal);
            text += L"\r\n";
            text += L"  Что сделать  :\r\n";
            text += joinLines(playbook.steps, L"    - ");
            text += L"  Команды      :\r\n";
            text += joinLines(playbook.commands, L"    - ");
            text += L"  На что смотреть:\r\n";
            text += joinLines(playbook.expectedSignals, L"    - ");
            if (!playbook.relatedTools.empty()) {
                text += L"  Куда ещё зайти:\r\n";
                text += joinLines(playbook.relatedTools, L"    - ");
            }
            text += L"\r\n";
        }
    }

    currentPlatform.clear();
    currentCategory.clear();
    if (!currentBundle_.liveCommandResults.empty()) {
        if (!text.empty()) {
            text += L"\r\n";
        }
        text += L"=== Живые результаты команд ===\r\n\r\n";
        for (const auto& result : currentBundle_.liveCommandResults) {
            if (result.platform != currentPlatform || result.category != currentCategory) {
                currentPlatform = result.platform;
                currentCategory = result.category;
                text += L"--- ";
                text += toWide(currentPlatform);
                text += L" :: ";
                text += toWide(currentCategory);
                text += L" ---\r\n\r\n";
            }

            text += (result.success ? L"[OK] " : L"[INFO] ");
            text += toWide(result.title);
            text += L"\r\n";
            if (!result.command.empty()) {
                text += L"  Команда      : ";
                text += toWide(result.command);
                text += L"\r\n";
            }
            text += L"  Для чего     : ";
            text += toWide(result.purpose);
            text += L"\r\n";
            text += L"  Примечание   : ";
            text += toWide(result.note);
            text += L"\r\n";
            text += L"  Результат:\r\n";
            text += L"    ------------------------------\r\n";
            text += joinLines(splitLines(result.output), L"    ");
            text += L"    ------------------------------\r\n\r\n";
        }
    }

    if (text.empty()) {
        text = L"Пока нет практических сценариев. Запусти режим обучения или кнопку \"Практика и тесты\".";
    }

    return text;
}

std::wstring WorkbenchWindow::buildToolsText() const {
    std::wstring text;
    std::string currentPlatform;
    std::string currentCategory;
    for (const auto& tool : currentBundle_.toolReferences) {
        if (tool.platform != currentPlatform || tool.category != currentCategory) {
            currentPlatform = tool.platform;
            currentCategory = tool.category;
            if (!text.empty()) {
                text += L"\r\n";
            }
            text += L"=== ";
            text += toWide(currentPlatform);
            text += L" :: ";
            text += toWide(currentCategory);
            text += L" ===\r\n\r\n";
        }

        text += toWide(tool.title);
        text += L"\r\n";
        text += L"  Где открыть  : ";
        text += toWide(tool.openTarget);
        text += L"\r\n";
        text += L"  Для чего     : ";
        text += toWide(tool.purpose);
        text += L"\r\n";
        text += L"  Когда идти   :\r\n";
        text += joinLines(tool.useCases, L"    - ");
        text += L"  Что смотреть :\r\n";
        text += joinLines(tool.whatToCheck, L"    - ");
        text += L"  Команды рядом:\r\n";
        text += joinLines(tool.relatedCommands, L"    - ");
        text += L"  Визуальный путь:\r\n";
        text += L"    Симптом -> ";
        text += toWide(tool.title);
        text += L" -> проверка -> интерпретация -> действие\r\n\r\n";
    }

    if (text.empty()) {
        text = L"В текущем наборе нет справочника по системным инструментам.";
    }
    return text;
}

std::wstring WorkbenchWindow::buildLocationsText() const {
    std::wstring text;
    std::string currentPlatform;
    std::string currentCategory;
    for (const auto& location : currentBundle_.artifactLocations) {
        if (location.platform != currentPlatform || location.category != currentCategory) {
            currentPlatform = location.platform;
            currentCategory = location.category;
            if (!text.empty()) {
                text += L"\r\n";
            }
            text += L"=== ";
            text += toWide(currentPlatform);
            text += L" :: ";
            text += toWide(currentCategory);
            text += L" ===\r\n\r\n";
        }

        text += toWide(location.artifact);
        text += L"\r\n";
        text += L"  Где искать:\r\n";
        text += joinLines(location.locations, L"    - ");
        text += L"  Команда      : ";
        text += toWide(location.inspectionCommand);
        text += L"\r\n";
        text += L"  Зачем смотреть: ";
        text += toWide(location.purpose);
        text += L"\r\n\r\n";
    }

    if (text.empty()) {
        text = L"В текущем наборе нет справки по расположению артефактов.";
    }
    return text;
}

std::wstring WorkbenchWindow::buildStudyText() const {
    std::wstring text;
    for (const auto& section : currentBundle_.learningSections) {
        text += toWide(section.audience);
        text += L" :: ";
        text += toWide(section.title);
        text += L"\r\n";
        text += L"Что изучить:\r\n";
        text += joinLines(section.topics, L"  - ");
        text += L"Практика:\r\n";
        text += joinLines(section.labs, L"  - ");
        text += L"Цели:\r\n";
        text += joinLines(section.milestones, L"  - ");
        text += L"\r\n";
    }

    if (text.empty()) {
        text = L"В текущем наборе нет учебного плана.";
    }
    return text;
}

std::wstring WorkbenchWindow::buildJsonText() const {
    return toWide(JsonWriter {}.stringify(currentBundle_));
}

void WorkbenchWindow::saveMarkdown() {
    if (currentBundle_.scenario.empty()) {
        MessageBoxW(hwnd_, L"Сначала запусти один из сценариев, а потом экспортируй отчет.", L"Windows Security Workbench", MB_OK | MB_ICONINFORMATION);
        return;
    }

    const auto path = promptSavePath(L"Сохранить Markdown-отчет", L"Markdown (*.md)\0*.md\0All files (*.*)\0*.*\0", L"md", toWide(currentBundle_.scenario + "-" + currentTimestampCompact()));
    if (!path.has_value()) {
        return;
    }

    ReportWriter {}.write(path.value(), currentBundle_);
    SetWindowTextW(statusLabel_, (L"Markdown сохранен: " + path->wstring()).c_str());
}

void WorkbenchWindow::saveJson() {
    if (currentBundle_.scenario.empty()) {
        MessageBoxW(hwnd_, L"Сначала запусти один из сценариев, а потом экспортируй отчет.", L"Windows Security Workbench", MB_OK | MB_ICONINFORMATION);
        return;
    }

    const auto path = promptSavePath(L"Сохранить JSON-отчет", L"JSON (*.json)\0*.json\0All files (*.*)\0*.*\0", L"json", toWide(currentBundle_.scenario + "-" + currentTimestampCompact()));
    if (!path.has_value()) {
        return;
    }

    JsonWriter {}.write(path.value(), currentBundle_);
    SetWindowTextW(statusLabel_, (L"JSON сохранен: " + path->wstring()).c_str());
}

std::optional<std::filesystem::path> WorkbenchWindow::promptSavePath(const wchar_t* title,
                                                                     const wchar_t* filter,
                                                                     const wchar_t* defaultExtension,
                                                                     const std::wstring& defaultName) const {
    std::array<wchar_t, MAX_PATH> fileBuffer {};
    wcsncpy_s(fileBuffer.data(), fileBuffer.size(), defaultName.c_str(), _TRUNCATE);

    OPENFILENAMEW fileName {};
    fileName.lStructSize = sizeof(fileName);
    fileName.hwndOwner = hwnd_;
    fileName.lpstrFilter = filter;
    fileName.lpstrFile = fileBuffer.data();
    fileName.nMaxFile = static_cast<DWORD>(fileBuffer.size());
    fileName.lpstrTitle = title;
    fileName.lpstrDefExt = defaultExtension;
    fileName.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileNameW(&fileName)) {
        return std::nullopt;
    }

    return std::filesystem::path(fileBuffer.data());
}

}  // namespace sec
