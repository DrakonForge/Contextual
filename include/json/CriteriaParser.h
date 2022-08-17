#pragma once

#include <memory>
#include <vector>

#include "JsonUtils.h"
#include "Response.h"
#include "RuleTable.h"
#include "StringTable.h"

namespace Contextual {

struct RuleInfo {
    std::vector<std::shared_ptr<Criteria>> criteria;
    std::shared_ptr<Response> response;
    int priority;
};

namespace CriteriaParser {

JsonParseResult parseCriteria(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, StringTable& stringTable,
                              const rapidjson::Value& root,
                              const std::unordered_map<std::string, RuleInfo>& namedRules);

}  // namespace CriteriaParser
}  // namespace Contextual