#pragma once

#include "CriteriaParser.h"
#include "DatabaseParser.h"
#include "JsonUtils.h"
#include "Response.h"
#include "RuleTable.h"
#include "SymbolToken.h"

namespace Contextual::RuleParser {

JsonParseResult parseRule(StringTable& stringTable, std::shared_ptr<RuleEntry>& rule,
                          std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId,
                          const rapidjson::Value& root, const std::string& idPrefix, DatabaseParser::ParsingType parsingType,
                          const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                          const std::unique_ptr<FunctionTable>& functionTable);

}  // namespace Contextual::RuleParser