#pragma once

#include <string>

#include "RuleDatabase.h"

namespace Contextual::RuleParser {

enum class RuleParserReturnCode : uint32_t {
    kSuccess = 0,
    kParentNotYetLoaded = 1,
    kInvalidPath = 10,
    kInvalidSyntax = 20,
    kInvalidType = 30,
    kInvalidValue = 40,
    kMissingKey = 50,
};

struct RuleParserResult {
    RuleParserReturnCode code;
    std::string errorMsg;
};

struct DatabaseStats {
    uint32_t numLoaded;
    uint32_t numFailed;
    uint32_t numTables;
    uint32_t numRules;
};

RuleParserResult loadGroup(RuleDatabase& out, const std::string& path);
DatabaseStats loadDatabase(RuleDatabase& out, const std::string& dirPath);
}