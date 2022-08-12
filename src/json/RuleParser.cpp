#include "RuleParser.h"

#include <iostream>

#include "CriterionAlternate.h"
#include "CriterionDynamic.h"
#include "CriterionEmpty.h"
#include "CriterionExist.h"
#include "CriterionFail.h"
#include "CriterionIncludes.h"
#include "CriterionStatic.h"
#include "response/ResponseMultiple.h"
#include "response/ResponseSimple.h"
#include "ResponseSpeech.h"
#include "SpeechTokenizer.h"
#include "SymbolParser.h"

namespace Contextual::RuleParser {

namespace {

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_CRITERIA = "Criteria";
const std::string g_KEY_RESPONSE = "Response";
const std::string g_KEY_TABLE = "Table";
const std::string g_KEY_KEY = "Key";
const std::string g_KEY_INVERT = "Invert";
const std::string g_KEY_CRITERION_TYPE = "Type";
const std::string g_KEY_CRITERION_VALUE = "Value";
const std::string g_KEY_RESPONSE_TYPE = "Type";
const std::string g_KEY_RESPONSE_VALUE = "Value";

const std::string g_RESPONSE_RANDOM = "Random";

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

// Cache some common criterion to prevent repeated objects
const std::shared_ptr<CriterionStatic> g_CRITERION_EQUALS_TRUE =
    std::make_shared<CriterionStatic>(-g_EPSILON, g_EPSILON, true);
const std::shared_ptr<CriterionStatic> g_CRITERION_EQUALS_FALSE =
    std::make_shared<CriterionStatic>(-g_EPSILON, g_EPSILON, false);
const std::shared_ptr<CriterionExist> g_CRITERION_EXISTS_TRUE = std::make_shared<CriterionExist>(false);
const std::shared_ptr<CriterionExist> g_CRITERION_EXISTS_FALSE = std::make_shared<CriterionExist>(true);
const std::shared_ptr<CriterionEmpty> g_CRITERION_EMPTY_TRUE = std::make_shared<CriterionEmpty>(false);
const std::shared_ptr<CriterionEmpty> g_CRITERION_EMPTY_FALSE = std::make_shared<CriterionEmpty>(true);

enum class DynamicComparisonType { kEquals, kLessThan, kLessEqual, kGreaterThan, kGreaterEqual };

JsonParseResult getTableKey(std::string& table, std::string& key, const rapidjson::Value& root) {
    if (!root.HasMember(g_KEY_TABLE)) {
        return {JsonParseReturnCode::kMissingKey, "Context value must specify key \"" + g_KEY_TABLE + "\""};
    }
    if (!root.HasMember(g_KEY_KEY)) {
        return {JsonParseReturnCode::kMissingKey, "Context value must specify key \"" + g_KEY_KEY + "\""};
    }
    auto result = JsonUtils::getString(table, root, g_KEY_TABLE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    result = JsonUtils::getString(key, root, g_KEY_KEY);
    return result;
}

JsonParseResult getInvert(bool& invert, const rapidjson::Value& root) {
    if (root.HasMember(g_KEY_INVERT)) {
        const auto& value = root[g_KEY_INVERT];
        if (!value.IsBool()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_INVERT + "\" must be a boolean"};
        }
        invert = value.GetBool();
    } else {
        invert = false;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

// Precondition: value is an object
JsonParseResult parseDynamicCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                      const rapidjson::Value& value, const std::string& table, const std::string& key,
                                      const bool invert, const DynamicComparisonType type) {
    std::string otherTable;
    std::string otherKey;
    auto result = getTableKey(otherTable, otherKey, value);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    float minDelta;
    float maxDelta;
    switch (type) {
        case DynamicComparisonType::kEquals:
            minDelta = -g_EPSILON;
            maxDelta = +g_EPSILON;
            break;
        case DynamicComparisonType::kGreaterEqual:
            minDelta = -g_EPSILON;
            maxDelta = g_INFINITY;
            break;
        case DynamicComparisonType::kGreaterThan:
            minDelta = g_EPSILON;
            maxDelta = g_INFINITY;
            break;
        case DynamicComparisonType::kLessEqual:
            minDelta = -g_INFINITY;
            maxDelta = g_EPSILON;
            break;
        case DynamicComparisonType::kLessThan:
            minDelta = -g_INFINITY;
            maxDelta = -g_EPSILON;
            break;
        default:
            return {JsonParseReturnCode::kInvalidValue, "Invalid dynamic comparison type"};
    }

    criteria.push_back(std::make_shared<Criteria>(
        table, key,
        std::make_shared<CriterionDynamic>(minDelta, maxDelta, std::move(otherTable), std::move(otherKey), invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseExistsCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                     const std::string& table, const std::string& key, const bool invert) {
    if (invert) {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EXISTS_FALSE));
    } else {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EXISTS_TRUE));
    }
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseEmptyCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                    const std::string& table, const std::string& key, const bool invert) {
    if (invert) {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EMPTY_FALSE));
    } else {
        criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EMPTY_TRUE));
    }
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseEqualsCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                     StringTable& stringTable, const rapidjson::Value& value, const std::string& table,
                                     const std::string& key, const bool invert) {
    if (value.IsObject()) {
        return parseDynamicCriterion(criteria, priority, value, table, key, invert, DynamicComparisonType::kEquals);
    }
    if (value.IsString()) {
        const std::string& strValue = value.GetString();
        auto cachedValue = static_cast<float>(stringTable.cache(strValue));
        criteria.push_back(std::make_shared<Criteria>(
            table, key, std::make_shared<CriterionStatic>(cachedValue - g_EPSILON, cachedValue + g_EPSILON, invert)));
    } else if (value.IsNumber()) {
        float numValue = value.GetFloat();
        criteria.push_back(std::make_shared<Criteria>(
            table, key, std::make_shared<CriterionStatic>(numValue - g_EPSILON, numValue + g_EPSILON, invert)));
    } else if (value.IsBool()) {
        if (value.GetBool() != invert) {
            criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EQUALS_TRUE));
        } else {
            criteria.push_back(std::make_shared<Criteria>(table, key, g_CRITERION_EQUALS_FALSE));
        }
    } else if (value.IsArray()) {
        // List of primitives
        std::unordered_set<int> options;
        options.reserve(value.Size());
        for (auto iter = value.Begin(); iter != value.Begin(); ++iter) {
            const auto& option = *iter;
            if (option.IsNumber()) {
                options.insert(option.GetInt());
            } else if (option.IsBool()) {
                options.insert(option.GetBool());
            } else if (option.IsString()) {
                int cachedValue = stringTable.cache(option.GetString());
                options.insert(cachedValue);
            } else {
                return {JsonParseReturnCode::kInvalidType,
                        "Alternate criterion options must be a numerical value, booleans, or strings"};
            }
        }
        criteria.push_back(
            std::make_shared<Criteria>(table, key, std::make_shared<CriterionAlternate>(std::move(options), invert)));
    } else {
        return {JsonParseReturnCode::kInvalidValue,
                "Unsupported value type for equals criterion: \"" + std::to_string(value.GetType()) + "\""};
    }

    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseLessThanCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                       const rapidjson::Value& value, const std::string& table, const std::string& key,
                                       const bool invert, bool equals) {
    if (value.IsObject()) {
        DynamicComparisonType type = equals ? DynamicComparisonType::kLessEqual : DynamicComparisonType::kLessThan;
        return parseDynamicCriterion(criteria, priority, value, table, key, invert, type);
    }
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Comparison criterion must have a numeric value"};
    }
    float max = value.GetFloat();
    if (equals) {
        max += g_EPSILON;
    } else {
        max -= g_EPSILON;
    }
    criteria.push_back(
        std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(-g_INFINITY, max, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseGreaterThanCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                          const rapidjson::Value& value, const std::string& table,
                                          const std::string& key, const bool invert, bool equals) {
    if (value.IsObject()) {
        DynamicComparisonType type =
            equals ? DynamicComparisonType::kGreaterEqual : DynamicComparisonType::kGreaterThan;
        return parseDynamicCriterion(criteria, priority, value, table, key, invert, type);
    }
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Comparison criterion must have a numeric value"};
    }
    float min = value.GetFloat();
    if (equals) {
        min -= g_EPSILON;
    } else {
        min += g_EPSILON;
    }
    criteria.push_back(
        std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(min, g_INFINITY, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseRangeCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                    const rapidjson::Value& value, const std::string& table, const std::string& key,
                                    const bool invert, const bool equalsMin, const bool equalsMax) {
    if (!value.IsArray() || value.Size() != 2 || !value[0].IsNumber() || !value[1].IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Range criterion must have an array of exactly 2 numeric values"};
    }
    float min = value[0].GetFloat();
    float max = value[1].GetFloat();

    if (equalsMin) {
        min -= g_EPSILON;
    } else {
        min += g_EPSILON;
    }

    if (equalsMax) {
        max += g_EPSILON;
    } else {
        max -= g_EPSILON;
    }

    criteria.push_back(std::make_shared<Criteria>(table, key, std::make_shared<CriterionStatic>(min, max, invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseIncludesCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                       StringTable& stringTable, const rapidjson::Value& value,
                                       const std::string& table, const std::string& key, const bool invert) {
    if (!value.IsArray() || value.Empty()) {
        return {JsonParseReturnCode::kInvalidType, "Includes criterion must have a nonempty array value"};
    }

    std::vector<int> options;
    options.reserve(value.Size());
    for (auto iter = value.Begin(); iter != value.End(); ++iter) {
        const auto& option = *iter;
        if (option.IsNumber()) {
            options.push_back(option.GetInt());
        } else if (option.IsBool()) {
            options.push_back(option.GetBool());
        } else if (option.IsString()) {
            int cachedValue = stringTable.cache(option.GetString());
            options.push_back(cachedValue);
        } else {
            return {JsonParseReturnCode::kInvalidType,
                    "Includes criterion options must be a numerical value, booleans, or strings"};
        }
    }

    criteria.push_back(
        std::make_shared<Criteria>(table, key, std::make_shared<CriterionIncludes>(std::move(options), invert)));
    ++priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseNamedCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                    const rapidjson::Value& value,
                                    const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if (!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Named criterion must have a string value"};
    }
    const std::string& name = value.GetString();
    auto got = namedRules.find(name);
    if (got == namedRules.end()) {
        return {JsonParseReturnCode::kMissingRule, "Rule with name \"" + name + "\" does not exist in this hierarchy"};
    }
    const RuleInfo& ruleInfo = got->second;
    criteria.insert(criteria.end(), ruleInfo.criteria.begin(), ruleInfo.criteria.end());
    priority += ruleInfo.priority;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseDummyCriterion(int& priority, const rapidjson::Value& value) {
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Dummy criterion must have a numeric value"};
    }
    int toAdd = value.GetInt();
    priority += toAdd;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFailCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, const rapidjson::Value& value) {
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Fail criterion must have a numeric value"};
    }
    float numValue = value.GetFloat();
    if (numValue < 0 || numValue > 1) {
        return {JsonParseReturnCode::kInvalidType, "Fail criterion must have a numeric value within the range [0, 1]"};
    }

    // Table and key should be unused
    criteria.push_back(std::make_shared<Criteria>("", "", std::make_shared<CriterionFail>(numValue)));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSpecialCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                                      const std::string& type, const rapidjson::Value& root,
                                      const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if (!root.HasMember(g_KEY_CRITERION_VALUE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_VALUE + "\""};
    }
    const auto& value = root[g_KEY_CRITERION_VALUE];

    // Fail, Dummy, and Named type do not need a table/key/invert
    if (type == g_TYPE_NAMED) {
        return parseNamedCriterion(criteria, priority, value, namedRules);
    }
    if (type == g_TYPE_DUMMY) {
        return parseDummyCriterion(priority, value);
    }
    if (type == g_TYPE_FAIL) {
        return parseFailCriterion(criteria, value);
    }
    return {JsonParseReturnCode::kInvalidValue, "Unrecognized special criterion type \"" + type + "\""};
}

JsonParseResult parseCriterion(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority,
                               StringTable& stringTable, const rapidjson::Value& root,
                               const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Criterion must be a JSON object"};
    }

    // Get type
    if (!root.HasMember(g_KEY_CRITERION_TYPE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_TYPE + "\""};
    }
    std::string type;
    auto result = JsonUtils::getString(type, root, g_KEY_CRITERION_TYPE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Special types that do not need context info
    if (g_SPECIAL_TYPES.find(type) != g_SPECIAL_TYPES.end()) {
        return parseSpecialCriterion(criteria, priority, type, root, namedRules);
    }

    std::string table;
    std::string key;
    result = getTableKey(table, key, root);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    bool invert;
    result = getInvert(invert, root);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Exists and empty type do not need a value
    if (type == g_TYPE_EXISTS) {
        return parseExistsCriterion(criteria, priority, table, key, invert);
    }
    if (type == g_TYPE_EMPTY) {
        return parseEmptyCriterion(criteria, priority, table, key, invert);
    }

    // Get value
    if (!root.HasMember(g_KEY_CRITERION_VALUE)) {
        return {JsonParseReturnCode::kMissingKey, "Criterion must specify key \"" + g_KEY_CRITERION_VALUE + "\""};
    }
    const auto& value = root[g_KEY_CRITERION_VALUE];

    if (type == g_TYPE_EQUALS) {
        return parseEqualsCriterion(criteria, priority, stringTable, value, table, key, invert);
    }
    if (type == g_TYPE_LESS_THAN) {
        return parseLessThanCriterion(criteria, priority, value, table, key, invert, false);
    }
    if (type == g_TYPE_LESS_EQUAL) {
        return parseLessThanCriterion(criteria, priority, value, table, key, invert, true);
    }
    if (type == g_TYPE_GREATER_THAN) {
        return parseGreaterThanCriterion(criteria, priority, value, table, key, invert, false);
    }
    if (type == g_TYPE_GREATER_EQUAL) {
        return parseGreaterThanCriterion(criteria, priority, value, table, key, invert, true);
    }
    if (type == g_TYPE_LT_LT) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, false, false);
    }
    if (type == g_TYPE_LT_LE) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, false, true);
    }
    if (type == g_TYPE_LE_LT) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, true, false);
    }
    if (type == g_TYPE_LE_LE) {
        return parseRangeCriterion(criteria, priority, value, table, key, invert, true, true);
    }
    if (type == g_TYPE_INCLUDES) {
        return parseIncludesCriterion(criteria, priority, stringTable, value, table, key, invert);
    }
    return {JsonParseReturnCode::kInvalidValue, "Unrecognized criterion type \"" + type + "\""};
}

JsonParseResult parseCriteria(std::vector<std::shared_ptr<Criteria>>& criteria, int& priority, StringTable& stringTable,
                              const rapidjson::Value& root,
                              const std::unordered_map<std::string, RuleInfo>& namedRules) {
    if (root.HasMember(g_KEY_CRITERIA)) {
        const auto& value = root[g_KEY_CRITERIA];
        if (!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CRITERIA + "\" must be an array"};
        }
        for (auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseCriterion(criteria, priority, stringTable, *iter, namedRules);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSpeechResponse(std::shared_ptr<Response>& response, const rapidjson::Value& value,
                                    const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                    const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                                    const std::unique_ptr<FunctionTable>& functionTable) {
    if (!value.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Speech response value must be an array"};
    }
    std::vector<std::vector<std::shared_ptr<SpeechToken>>> speechLines;
    speechLines.reserve(value.Size());

    for (auto iter = value.Begin(); iter != value.End(); ++iter) {
        const auto& line = *iter;
        if (!line.IsString()) {
            return {JsonParseReturnCode::kInvalidType, "All values in a speech response array must be a string"};
        }
        std::vector<std::shared_ptr<SpeechToken>> speechLine;
        const std::string& lineStr = line.GetString();
        auto result = SpeechTokenizer::tokenize(speechLine, lineStr, symbols, localSymbols, functionTable);
        if(result.code != SpeechTokenizerReturnCode::kSuccess) {
            return {JsonParseReturnCode::kInvalidValue, "Failed to parse speech line \"" + lineStr + "\": " + result.errorMsg};
        }
        speechLines.push_back(speechLine);
    }

    response = std::make_shared<ResponseSpeech>(std::move(speechLines));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSimpleResponse(std::shared_ptr<Response>& response, const rapidjson::Value& value) {
    if (!value.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Simple response value must be an array"};
    }
    std::vector<std::string> options;
    options.reserve(value.Size());
    for (auto iter = value.Begin(); iter != value.End(); ++iter) {
        const auto& option = *iter;
        if (!option.IsString()) {
            return {JsonParseReturnCode::kInvalidType, "All values in a simple response array must be a string"};
        }
        options.emplace_back(option.GetString());
    }
    response = std::make_shared<ResponseSimple>(std::move(options));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseResponseObject(std::shared_ptr<Response>& response, const rapidjson::Value& root,
                                    const ParsingType parsingType,
                                    const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                    const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                                    const std::unique_ptr<FunctionTable>& functionTable) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Response must be a JSON object"};
    }
    if (!root.HasMember(g_KEY_RESPONSE_TYPE)) {
        return {JsonParseReturnCode::kInvalidType, "Response must specify key \"" + g_KEY_RESPONSE_TYPE + "\""};
    }
    if (!root.HasMember(g_KEY_RESPONSE_VALUE)) {
        return {JsonParseReturnCode::kInvalidType, "Response must specify key \"" + g_KEY_RESPONSE_VALUE + "\""};
    }
    std::string type;
    auto result = JsonUtils::getString(type, root, g_KEY_RESPONSE_TYPE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    const auto& value = root[g_KEY_RESPONSE_VALUE];

    if (type == g_RESPONSE_RANDOM) {
        if (parsingType == ParsingType::kSpeechbank) {
            return parseSpeechResponse(response, value, symbols, localSymbols, functionTable);
        }
        return parseSimpleResponse(response, value);
    }

    // TODO: For any non-random response, check parsing type
    return {JsonParseReturnCode::kInvalidValue, "Unknown response type \"" + g_KEY_RESPONSE_TYPE + "\""};
}

JsonParseResult parseResponse(std::shared_ptr<Response>& response, const rapidjson::Value& root,
                              const ParsingType parsingType,
                              const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                              const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                              const std::unique_ptr<FunctionTable>& functionTable) {
    if (root.HasMember(g_KEY_RESPONSE)) {
        const auto& value = root[g_KEY_RESPONSE];
        if (!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_RESPONSE + "\" must be an array"};
        }
        if (value.Empty()) {
            return {JsonParseReturnCode::kInvalidValue, "Key \"" + g_KEY_RESPONSE + "\" must be a nonempty array"};
        }
        if (value.Size() == 1) {
            // Single response
            return parseResponseObject(response, value[0], parsingType, symbols, localSymbols, functionTable);
        } else {
            std::vector<std::shared_ptr<Response>> responses;
            for (auto iter = value.Begin(); iter != value.End(); ++iter) {
                std::shared_ptr<Response> item;
                auto result = parseResponseObject(item, *iter, parsingType, symbols, localSymbols, functionTable);
                if (result.code != JsonParseReturnCode::kSuccess) {
                    return result;
                }
                responses.push_back(item);
            }
            response = std::make_shared<ResponseMultiple>(std::move(responses));
            return JsonUtils::g_RESULT_SUCCESS;
        }
    }
    response = nullptr;
    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace

JsonParseResult parseRule(StringTable& stringTable, std::unique_ptr<RuleEntry>& rule,
                          std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId,
                          const rapidjson::Value& root, const std::string& idPrefix, const ParsingType parsingType,
                          const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                          const std::unique_ptr<FunctionTable>& functionTable) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Rule must be a JSON object"};
    }

    int priority = 0;
    std::vector<std::shared_ptr<Criteria>> criteria;
    auto result = parseCriteria(criteria, priority, stringTable, root, namedRules);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Reads local symbols from key "Symbols" if it exists
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>> localSymbols;
    result = SymbolParser::parseSymbols(localSymbols, root, symbols, functionTable);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::shared_ptr<Response> response;
    result = parseResponse(response, root, parsingType, symbols, localSymbols, functionTable);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::string id;
    if (root.HasMember(g_KEY_NAME)) {
        std::string name;
        result = JsonUtils::getString(name, root, g_KEY_NAME);
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        if (namedRules.find(name) != namedRules.end()) {
            return {JsonParseReturnCode::kAlreadyDefined,
                    "Rule with name \"" + name + "\" already exists in this hierarchy"};
        }
        id = idPrefix + name;
        // Add named rule
        namedRules.insert({name, {criteria, response, priority}});
    } else {
        id = idPrefix + g_DEFAULT_ID + std::to_string(nextId);
        ++nextId;
    }

    // Rule has no response, do not create entry for it
    if (response == nullptr) {
        return {JsonParseReturnCode::kSkipCreation, ""};
    }

    rule = std::make_unique<RuleEntry>();
    rule->id = id;
    rule->criteria = criteria;
    rule->priority = priority;
    rule->response = response;
    return JsonUtils::g_RESULT_SUCCESS;
}
}  // namespace Contextual::RuleParser