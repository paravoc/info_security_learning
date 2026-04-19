#include "modules/cleanup/CleanupService.h"

#include "platform/windows/Encoding.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string>

namespace {

struct ScanSummary {
    std::uint64_t files = 0;
    std::uint64_t bytes = 0;
};

std::filesystem::path queryUserTempPath() {
    std::wstring buffer(MAX_PATH, L'\0');
    const auto length = GetTempPathW(static_cast<DWORD>(buffer.size()), buffer.data());
    buffer.resize(length);
    return std::filesystem::path(buffer);
}

std::filesystem::path queryWindowsTempPath() {
    std::wstring buffer(MAX_PATH, L'\0');
    const auto length = GetWindowsDirectoryW(buffer.data(), static_cast<UINT>(buffer.size()));
    buffer.resize(length);
    return std::filesystem::path(buffer) / "Temp";
}

ScanSummary scanPath(const std::filesystem::path& root) {
    ScanSummary summary;
    std::error_code error;

    if (!std::filesystem::exists(root, error)) {
        return summary;
    }

    for (std::filesystem::recursive_directory_iterator it(root,
                                                           std::filesystem::directory_options::skip_permission_denied,
                                                           error);
         it != std::filesystem::recursive_directory_iterator();
         it.increment(error)) {
        if (error) {
            error.clear();
            continue;
        }

        const auto& entry = *it;
        if (entry.is_regular_file(error)) {
            ++summary.files;
            summary.bytes += static_cast<std::uint64_t>(entry.file_size(error));
        }
    }

    return summary;
}

std::string cleanDirectory(const std::filesystem::path& root, const bool dryRun, std::uint64_t& removedObjects) {
    std::error_code error;
    if (!std::filesystem::exists(root, error)) {
        return "Цель очистки не существует.";
    }

    if (dryRun) {
        return "Только dry-run. Файлы не удалялись.";
    }

    std::uint64_t failures = 0;
    for (const auto& entry : std::filesystem::directory_iterator(root, std::filesystem::directory_options::skip_permission_denied, error)) {
        if (error) {
            ++failures;
            error.clear();
            continue;
        }

        std::error_code removeError;
        removedObjects += static_cast<std::uint64_t>(std::filesystem::remove_all(entry.path(), removeError));
        if (removeError) {
            ++failures;
        }
    }

    if (failures == 0) {
        return "Очистка завершена.";
    }

    return "Очистка завершена с " + std::to_string(failures) + " заблокированными или недоступными элементами.";
}

sec::CleanupTargetResult cleanTempTarget(const std::string& label, const std::filesystem::path& path, const bool dryRun) {
    sec::CleanupTargetResult result;
    result.target = label + " (" + sec::toUtf8(path.wstring()) + ")";

    const auto scan = scanPath(path);
    result.filesAffected = scan.files;
    result.bytesAffected = scan.bytes;

    std::uint64_t removedObjects = 0;
    result.message = cleanDirectory(path, dryRun, removedObjects);
    result.success = true;
    return result;
}

sec::CleanupTargetResult cleanRecycleBin(const bool dryRun) {
    sec::CleanupTargetResult result;
    result.target = "Корзина";

    SHQUERYRBINFO info {};
    info.cbSize = sizeof(info);
    const auto queryStatus = SHQueryRecycleBinW(nullptr, &info);
    if (queryStatus != S_OK) {
        result.success = false;
        result.message = "Не удалось проверить корзину.";
        return result;
    }

    result.filesAffected = static_cast<std::uint64_t>(info.i64NumItems);
    result.bytesAffected = static_cast<std::uint64_t>(info.i64Size);

    if (dryRun) {
        result.success = true;
        result.message = "Только dry-run. Корзина не очищалась.";
        return result;
    }

    const auto emptyStatus = SHEmptyRecycleBinW(nullptr, nullptr, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
    result.success = emptyStatus == S_OK;
    result.message = result.success ? "Корзина очищена." : "Очистка корзины завершилась ошибкой.";
    return result;
}

}  // namespace

namespace sec {

CleanupSummary CleanupService::run(const bool dryRun) const {
    CleanupSummary summary;
    summary.dryRun = dryRun;

    summary.targets.push_back(cleanTempTarget("Пользовательский temp", queryUserTempPath(), dryRun));
    summary.targets.push_back(cleanTempTarget("Системный temp", queryWindowsTempPath(), dryRun));
    summary.targets.push_back(cleanRecycleBin(dryRun));

    return summary;
}

}  // namespace sec
