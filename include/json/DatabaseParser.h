#pragma once

#include <string>

#include "JsonUtils.h"
#include "RuleDatabase.h"

namespace Contextual::DatabaseParser {

struct DatabaseStats {
    uint32_t numLoaded;
    uint32_t numFailed;
    uint32_t numTables;
    uint32_t numRules;
};

enum class ParsingType : uint8_t { kDefault, kSpeechbank, kSimple };

JsonParseResult loadGroup(RuleDatabase& out, const std::string& path);
DatabaseStats loadDatabase(RuleDatabase& out, const std::string& dirPath);
}  // namespace Contextual::RuleParser