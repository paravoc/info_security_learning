#include "history/HistoryStore.h"

#include "core/TimeUtils.h"
#include "report/JsonWriter.h"

#include <filesystem>

namespace sec {

std::optional<std::filesystem::path> HistoryStore::save(const ExecutionBundle& bundle) const {
    const auto historyDirectory = std::filesystem::path("history");
    std::filesystem::create_directories(historyDirectory);

    const auto snapshotPath = historyDirectory / (currentTimestampCompact() + "-" + bundle.scenario + ".json");
    JsonWriter {}.write(snapshotPath, bundle);
    JsonWriter {}.write(historyDirectory / "latest.json", bundle);

    return snapshotPath;
}

}  // namespace sec
