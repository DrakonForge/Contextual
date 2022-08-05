#include "RuleParser.h"

#include <iostream>

#include "CriterionExist.h"
#include "CriterionFail.h"
#include "CriterionStatic.h"

namespace Contextual::RuleParser {

namespace {

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_CRITERIA = "Criteria";
const std::string g_KEY_SYMBOLS = "Symbols";
const std::string g_KEY_RESPONSE = "Response";
const std::string g_KEY_TABLE = "Table";
const std::string g_KEY_KEY = "Key";
const std::string g_KEY_INVERT = "Invert";

const std::string g_KEY_CRITERION_TYPE = "Type";
const std::string g_KEY_CRITERION_VALUE = "Value";

const std::string g_TYPE_EQUALS = "Eq";
const std::string g_TYPE_LESS_THAN = "Lt";
const std::string g_TYPE_LESS_EQUAL = "Le";
const std::string g_TYPE_GREATER_THAN = "Gt";
const std::string g_TYPE_GREATER_EQUAL = "Ge";
const std::string g_TYPE_LT_LT = "LtLt";
const std::string g_TYPE_LT_LE = "LtLe";
const std::string g_TYPE_LE_LT = "LeLt";
const std::string g_TYPE_LE_LE = "LeLe";
const std::string g_TYPE_EXISTS = "Exists";
// List stuff
const std::string g_TYPE_INCLUDES = "Includes";
const std::string g_TYPE_EMPTY = "Empty";
// Special
const std::string g_TYPE_NAMED = "Named";
const std::string g_TYPE_DUMMY = "Dummy";
const std::string g_TYPE_FAIL = "Fail";

const std::unordered_set<std::string> g_SPECIAL_TYPES = {g_TYPE_NAMED, g_TYPE_DUMMY, g_TYPE_FAIL};
const std::string g_DEFAULT_ID = "Id";
const float g_EPSILON = std::numeric_limits<float>::epsilon();
const float g_INFINITY = std::numeric_limits<float>::infinity();

const std::shared_ptr<CriterionStatic> g_CRITERION_EQUALS_TRUE = std::make_shared<CriterionStatic>(-g_EPSILON, g_EPSILON, true);
const std::shared_ptr<CriterionStatic> g_CRITERION_EQUALS_FALSE = std::make_shared<CriterionStatic>(-g_EPSILON, g_EPSILON, false);
const std::shared_ptr<CriterionExist> g_CRITERION_EXISTS_TRUE = std::make_shared<CriterionExist>(false);
const std::shared_ptr<CriterionExist> g_CRITERION_EXISTS_FALSE = std::make_shared<CriterionExist>(true);

JsonParseResult getTableKey(std::string& table, std::string& key, const rapidjson::Value& root) {
    if(!root.HasMember(g_KEY_TABLE)) {
        return {JsonParseReturnCode::kMissingKey, "Context value must specify key \"" + g_KEY_TABLE + "\""};
    }
    if(!root.HasMember(g_KEY_KEY)) {
        return {JsonParseReturnCode::kMissingKey, "Context value must specify key \"" + g_KEY_KEY + "\""};
    }
    auto result = JsonUtils::getString(table, root, g_KEY_TABLE);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    result = JsonUtils::getString(key, root, g_KEY_KEY);
    return result;
}

JsonParseResult getInvert(bool& invert, const rapidjson::Value& root) {
    if(root.HasMember(g_KEY_INVERT)) {
        const auto& value = root[g_KEY_INVERT];
        if(!value.IsBool()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_INVERT + "\" must be a boolean"};
        }
        invert = value.GetBool();
    } else {
        invert = false;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseEqualsCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, StringTable& stringTable, const rapidjson::Value& value, const std::string& table, const std::string& key, const bool invert) {
    if(value.IsString()) {
        const std::string& strValue = value.GetString();
        auto numValue = static_cast<float>(stringTable.cache(strValue));
        criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(numValue - g_EPSILON, numValue + g_EPSILON, invert)));
    } else if(value.IsNumber()) {
        float numValue = value.GetFloat();
        criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(numValue - g_EPSILON, numValue + g_EPSILON, invert)));
    } else if(value.IsBool()) {
        if(value.GetBool() != invert) {
            criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EQUALS_TRUE));
        } else {
            criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EQUALS_FALSE));
        }
    } else if(value.IsObject()) {
        // Dynamic
        // TODO CriterionDynamic
    } else if(value.IsArray()) {
        // List of primitives
        // TODO: CriterionAlternate
    } else {
        return {JsonParseReturnCode::kInvalidValue, "Unsupported value type for equals criterion: \"" + std::to_string(value.GetType()) + "\"" };
    }

    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseLessThanCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& value, const std::string& table, const std::string& key, const bool invert, bool equals) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Comparison criterion must have a numeric value"};
    }
    float max = value.GetFloat();
    if(equals) {
        max += g_EPSILON;
    } else {
        max -= g_EPSILON;
    }
    criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(-g_INFINITY, max, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseGreaterThanCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& value, const std::string& table, const std::string& key, const bool invert, bool equals) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Comparison criterion must have a numeric value"};
    }
    float min = value.GetFloat();
    if(equals) {
        min -= g_EPSILON;
    } else {
        min += g_EPSILON;
    }
    criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(min, g_INFINITY, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseRangeCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& value, const std::string& table, const std::string& key, const bool invert, const bool equalsMin, const bool equalsMax) {
    if(!value.IsArray() || value.Size() != 2 || !value[0].IsNumber() || !value[1].IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Range criterion must have an array of exactly 2 numeric values"};
    }
    float min = value[0].GetFloat();
    float max = value[1].GetFloat();

    if(equalsMin) {
        min -= g_EPSILON;
    } else {
        min += g_EPSILON;
    }

    if(equalsMax) {
        max += g_EPSILON;
    } else {
        max -= g_EPSILON;
    }

    criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(min, max, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseExistsCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& root, const std::string& table, const std::string& key, const bool invert) {
    if(invert) {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EXISTS_FALSE));
    } else {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EXISTS_TRUE));
    }
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseNamedCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& value, const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if(!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Named criterion must have a string value"};
    }
    const std::string& name = value.GetString();
    if(namedRules.find(name) == namedRules.end()) {
        return {JsonParseReturnCode::kMissingRule, "Rule with name \"" + name + "\" does not exist in this hierarchy"};
    }
    const RuleInfo& ruleInfo = namedRules.at(name);
    criteria.insert(criteria.end(), ruleInfo.criteria.begin(), ruleInfo.criteria.end());
    priority += ruleInfo.priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseDummyCriterion(int& priority, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Dummy criterion must have a numeric value"};
    }
    int toAdd = value.GetInt();
    priority += toAdd;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFailCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Fail criterion must have a numeric value"};
    }
    float numValue = value.GetFloat();
    if(numValue < 0 || numValue > 1) {
        return {JsonParseReturnCode::kInvalidType, "Fail criterion must have a numeric value within the range [0, 1]"};
    }

    // Table and key should be unused
    criteria.push_back(std::make_shared<Criteria>("", "", std::make_shared<CriterionFail>(numValue)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSpecialCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, const std::string& type, const rapidjson::Value& root, const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if(!root.HasMember(g_KEY_CRITERION_VALUE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_VALUE + "\""};
    }
    const auto& value = root[g_KEY_CRITERION_VALUE];

    // Fail, Dummy, and Named type do not need a table/key/invert
    if(type == g_TYPE_NAMED) {
        return parseNamedCriterion(criteria, priority, value, namedRules);
    }
    if(type == g_TYPE_DUMMY) {
        return parseDummyCriterion(priority, value);
    }
    if(type == g_TYPE_FAIL) {
        return parseFailCriterion(criteria, priority, value);
    }
    return {JsonParseReturnCode::kInvalidValue, "Unrecognized special criterion type \"" + type + "\""};
}

JsonParseResult parseCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, StringTable& stringTable, const rapidjson::Value& root, const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if(!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Criterion must be a JSON object"};
    }
    
    // Get type
    if(!root.HasMember(g_KEY_CRITERION_TYPE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_TYPE + "\""};
    }
    std::string type;
    auto result = JsonUtils::getString(type, root, g_KEY_CRITERION_TYPE);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Special types that do not need context info
    if(g_SPECIAL_TYPES.find(type) != g_SPECIAL_TYPES.end()) {
        return parseSpecialCriterion(criteria, priority, type, root, namedRules);
    }
    
    std::string table;
    std::string key;
    result = getTableKey(table, key, root);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    bool invert;
    result = getInvert(invert, root);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Exists type does not need a value
    if(type == g_TYPE_EXISTS) {
        return parseExistsCriterion(criteria, priority, root, table, key, invert);
    }

    // Get value
    if(!root.HasMember(g_KEY_CRITERION_VALUE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_VALUE + "\""};
    }
    const auto& value = root[g_KEY_CRITERION_VALUE];
    
    if(type == g_TYPE_EQUALS) {
        return parseEqualsCriterion(criteria, priority, stringTable, value, table, key, invert);
    }
    if(type == g_TYPE_LESS_THAN) {
        return parseLessThanCriterion(criteria, priority, value, table, key, invert, false);
    }
    if(type == g_TYPE_LESS_EQUAL) {
        return parseLessThanCriterion(criteria, priority, value, table, key, invert, true);
    }
    if(type == g_TYPE_GREATER_THAN) {
        return parseGreaterThanCriterion(criteria, priority, value, table, key, invert, false);
    }
    if(type == g_TYPE_GREATER_EQUAL) {
        return parseGreaterThanCriterion(criteria, priority, value, table, key, invert, true);
    }
    if(type == g_TYPE_LT_LT) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, false, false);
    }
    if(type == g_TYPE_LT_LE) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, false, true);
    }
    if(type == g_TYPE_LE_LT) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, true, false);
    }
    if(type == g_TYPE_LE_LE) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, true, true);
    }
    if(type == g_TYPE_INCLUDES) {
        // TODO Includes criterion
    }
    if(type == g_TYPE_EMPTY) {
        // TODO empty criterion
    }
    return {JsonParseReturnCode::kInvalidValue, "Unrecognized criterion type \"" + type + "\""};
}

JsonParseResult parseCriteria(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, StringTable& stringTable, const rapidjson::Value& root, const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if(root.HasMember(g_KEY_CRITERIA)) {
        const auto& value = root[g_KEY_CRITERIA];
        if(!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CRITERIA + "\" must be an array" };
        }
        for(auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseCriterion(criteria, priority, stringTable, *iter, namedRules);
            if(result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseResponse(std::shared_ptr<Response>& response, const rapidjson::Value& root) {
    if(root.HasMember(g_KEY_RESPONSE)) {

    } else {
        response = nullptr;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

}

JsonParseResult parseRule(
    StringTable& stringTable,
    std::unique_ptr<RuleEntry>& rule,
    std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId,
    const rapidjson::Value& root, const std::string& idPrefix, const ParsingType type,
    const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {

    if(!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Rule must be a JSON object" };
    }
    // TODO parse order: criteria, symbols, responses -> if none, then do not create rule (do not return success), name -> if name, then add to namedRules

    int priority = 0;
    std::vector<std::shared_ptr<Criteria>> criteria;
    auto result = parseCriteria(criteria, priority, stringTable, root, namedRules);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::shared_ptr<Response> response;
    result = parseResponse(response, root);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::string id;
    if(root.HasMember(g_KEY_NAME)) {
        std::string name;
        result = JsonUtils::getString(name, root, g_KEY_NAME);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        if(namedRules.find(name) != namedRules.end()) {
            return {JsonParseReturnCode::kAlreadyDefined, "Rule with name \"" + name + "\" already exists in this hierarchy" };
        }
        id = idPrefix + name;
        // Add named rule
        namedRules.insert({name, {criteria, response, priority}});
    } else {
        id = idPrefix + g_DEFAULT_ID + std::to_string(nextId);
        ++nextId;
    }

    // TODO: Temp code since responses are not parsed yet
    if(!root.HasMember(g_KEY_RESPONSE)) {
        return {JsonParseReturnCode::kSkipCreation, ""};
    }
    // Rule has no response, do not create entry for it
//    if(response == nullptr) {
//        return {JsonParseReturnCode::kSkipCreation, ""};
//    }

    rule = std::make_unique<RuleEntry>();
    rule->id = id;
    rule->criteria = criteria;
    rule->priority = priority;
    rule->response = response;
    return JsonUtils::g_RESULT_SUCCESS;
}
}