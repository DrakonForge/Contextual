#pragma once

#include "DatabaseParser.h"
#include "JsonUtils.h"
#include "response/Response.h"
#include "RuleTable.h"
#include "token/SymbolToken.h"

namespace Contextual::RuleParser {

struct RuleInfo {
    std::vector<std::shared_ptr<Criteria>> criteria;
    std::shared_ptr<Response> response;
    int priority;
};

JsonParseResult parseRule(StringTable& stringTable, std::unique_ptr<RuleEntry>& rule, std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId, const rapidjson::Value& root, const std::string& idPrefix, ParsingType parsingType, const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols);

}