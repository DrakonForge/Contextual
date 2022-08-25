#include "RuleParser.h"

#include "CriteriaParser.h"
#include "ResponseContextAdd.h"
#include "ResponseContextInvert.h"
#include "ResponseContextMultiply.h"
#include "ResponseContextSetDynamic.h"
#include "ResponseContextSetList.h"
#include "ResponseContextSetStatic.h"
#include "ResponseEvent.h"
#include "ResponseMultiple.h"
#include "ResponseSimple.h"
#include "ResponseSpeech.h"
#include "SpeechTokenizer.h"
#include "SymbolParser.h"

namespace Contextual::RuleParser {

namespace {

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_RESPONSE = "Response";
const std::string g_KEY_RESPONSE_TYPE = "Type";
const std::string g_KEY_RESPONSE_VALUE = "Value";
const std::string g_KEY_RESPONSE_EVENT_NAME = "Name";
const std::string g_KEY_RESPONSE_EVENT_ARGS = "Args";

const std::string g_RESPONSE_TEXT = "Text";
const std::string g_RESPONSE_EVENT = "Event";
const std::string g_RESPONSE_CONTEXT = "Context";
const std::string g_RESPONSE_CONTEXT_OP = "Op";
const std::string g_RESPONSE_CONTEXT_TABLE = "Table";
const std::string g_RESPONSE_CONTEXT_KEY = "Key";
const std::string g_RESPONSE_CONTEXT_VALUE = "Value";

const std::string g_OP_SET = "Set";
const std::string g_OP_INVERT = "Invert";
const std::string g_OP_ADD = "Add";
const std::string g_OP_SUB = "Sub";
const std::string g_OP_MULT = "Mult";
const std::string g_OP_DIV = "Div";

const std::string g_DEFAULT_ID = "Id";

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
        if (result.code != SpeechTokenizerReturnCode::kSuccess) {
            return {JsonParseReturnCode::kInvalidValue,
                    "Failed to parse speech line \"" + lineStr + "\": " + result.errorMsg};
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

JsonParseResult parseEventResponse(std::shared_ptr<Response>& response, const rapidjson::Value& value) {
    if (!value.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Event response value must be an object"};
    }

    if (!value.HasMember(g_KEY_RESPONSE_EVENT_NAME)) {
        return {JsonParseReturnCode::kMissingKey,
                "Event response value must specify key \"" + g_KEY_RESPONSE_EVENT_NAME + "\""};
    }
    if (!value.HasMember(g_KEY_RESPONSE_EVENT_ARGS)) {
        return {JsonParseReturnCode::kMissingKey,
                "Event response value must specify key \"" + g_KEY_RESPONSE_EVENT_ARGS + "\""};
    }
    std::string name;
    auto result = JsonUtils::getString(name, value, g_KEY_RESPONSE_EVENT_NAME);

    const auto& argValue = value[g_KEY_RESPONSE_EVENT_ARGS];
    if (!argValue.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Event response arguments must be an array"};
    }

    std::vector<std::string> args;
    for (auto iter = argValue.Begin(); iter != argValue.End(); ++iter) {
        const auto& arg = *iter;
        if (!arg.IsString()) {
            return {JsonParseReturnCode::kInvalidType, "All values in event response arguments must be a string"};
        }
        args.emplace_back(arg.GetString());
    }
    response = std::make_shared<ResponseEvent>(std::move(name), std::move(args));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseContextResponse(std::shared_ptr<Response>& response, StringTable& stringTable,
                                     const rapidjson::Value& root) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Context response value must be an object"};
    }
    if (!root.HasMember(g_RESPONSE_CONTEXT_OP)) {
        return {JsonParseReturnCode::kMissingKey,
                "Context response value must specify key \"" + g_RESPONSE_CONTEXT_OP + "\""};
    }
    if (!root.HasMember(g_RESPONSE_CONTEXT_TABLE)) {
        return {JsonParseReturnCode::kMissingKey,
                "Context response value must specify key \"" + g_RESPONSE_CONTEXT_TABLE + "\""};
    }
    if (!root.HasMember(g_RESPONSE_CONTEXT_KEY)) {
        return {JsonParseReturnCode::kMissingKey,
                "Context response value must specify key \"" + g_RESPONSE_CONTEXT_KEY + "\""};
    }
    std::string op;
    auto result = JsonUtils::getString(op, root, g_RESPONSE_CONTEXT_OP);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    std::string table;
    result = JsonUtils::getString(table, root, g_RESPONSE_CONTEXT_TABLE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    std::string key;
    result = JsonUtils::getString(key, root, g_RESPONSE_CONTEXT_KEY);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Invert does not require a value
    if (op == g_OP_INVERT) {
        response = std::make_shared<ResponseContextInvert>(std::move(table), std::move(key));
        return JsonUtils::g_RESULT_SUCCESS;
    }

    if (!root.HasMember(g_RESPONSE_CONTEXT_VALUE)) {
        return {JsonParseReturnCode::kMissingKey,
                "Context response value must specify key \"" + g_RESPONSE_CONTEXT_VALUE + "\""};
    }
    const auto& value = root[g_RESPONSE_CONTEXT_VALUE];

    if (op == g_OP_SET) {
        if (value.IsBool()) {
            response = std::make_shared<ResponseContextSetStatic>(std::move(table), std::move(key), FactType::kBoolean,
                                                                  value.GetBool());
        } else if (value.IsNumber()) {
            response = std::make_shared<ResponseContextSetStatic>(std::move(table), std::move(key), FactType::kNumber,
                                                                  value.GetFloat());
        } else if (value.IsString()) {
            response = std::make_shared<ResponseContextSetStatic>(std::move(table), std::move(key), FactType::kString,
                                                                  stringTable.cache(value.GetString()));
        } else if (value.IsObject()) {
            // Set to another context key
            if (!value.HasMember(g_RESPONSE_CONTEXT_TABLE)) {
                return {JsonParseReturnCode::kMissingKey,
                        "Context response set context value must specify key \"" + g_RESPONSE_CONTEXT_TABLE + "\""};
            }
            if (!value.HasMember(g_RESPONSE_CONTEXT_KEY)) {
                return {JsonParseReturnCode::kMissingKey,
                        "Context response set context value must specify key \"" + g_RESPONSE_CONTEXT_KEY + "\""};
            }
            std::string otherTable;
            result = JsonUtils::getString(otherTable, value, g_RESPONSE_CONTEXT_TABLE);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
            std::string otherKey;
            result = JsonUtils::getString(otherKey, value, g_RESPONSE_CONTEXT_KEY);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
            response = std::make_shared<ResponseContextSetDynamic>(std::move(table), std::move(key),
                                                                   std::move(otherTable), std::move(otherKey));
        } else if (value.IsArray()) {
            // Set list
            std::unordered_set<int> list;
            list.reserve(value.Size());
            bool isStringList = true;
            for (auto iter = value.Begin(); iter != value.End(); ++iter) {
                const auto& item = *iter;
                if (item.IsString()) {
                    list.insert(stringTable.cache(item.GetString()));
                } else if (item.IsNumber()) {
                    list.insert(item.GetInt());
                    isStringList = false;
                } else {
                    return {JsonParseReturnCode::kInvalidType,
                            "Context response set list item must be a string or integer"};
                }
            }
            response = std::make_shared<ResponseContextSetList>(std::move(table), std::move(key), std::move(list),
                                                                isStringList);
        } else {
            return {JsonParseReturnCode::kInvalidValue, "Unsupported value type for context response set operation"};
        }
        return JsonUtils::g_RESULT_SUCCESS;
    }

    if (op == g_OP_ADD) {
        if (!value.IsNumber()) {
            return {JsonParseReturnCode::kInvalidType, "Value for add operation must be numeric"};
        }
        response = std::make_shared<ResponseContextAdd>(std::move(table), std::move(key), value.GetFloat());
        return JsonUtils::g_RESULT_SUCCESS;
    }

    if (op == g_OP_SUB) {
        if (!value.IsNumber()) {
            return {JsonParseReturnCode::kInvalidType, "Value for subtract operation must be numeric"};
        }
        response = std::make_shared<ResponseContextAdd>(std::move(table), std::move(key), -value.GetFloat());
        return JsonUtils::g_RESULT_SUCCESS;
    }

    if (op == g_OP_MULT) {
        if (!value.IsNumber()) {
            return {JsonParseReturnCode::kInvalidType, "Value for multiply operation must be numeric"};
        }
        response = std::make_shared<ResponseContextAdd>(std::move(table), std::move(key), value.GetFloat());
        return JsonUtils::g_RESULT_SUCCESS;
    }
    if (op == g_OP_DIV) {
        if (!value.IsNumber()) {
            return {JsonParseReturnCode::kInvalidType, "Value for multiply operation must be numeric"};
        }
        float floatVal = value.GetFloat();
        if (floatVal == 0.0f) {
            return {JsonParseReturnCode::kInvalidValue, "Divide operation cannot divide by zero"};
        }
        response = std::make_shared<ResponseContextAdd>(std::move(table), std::move(key), 1.0f / floatVal);
        return JsonUtils::g_RESULT_SUCCESS;
    }
    return {JsonParseReturnCode::kInvalidValue, "Unknown context response operation \"" + op + "\""};
}

JsonParseResult parseResponseObject(std::shared_ptr<Response>& response, StringTable& stringTable,
                                    const rapidjson::Value& root, const DatabaseParser::ParsingType parsingType,
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

    if (type == g_RESPONSE_TEXT) {
        if (parsingType == DatabaseParser::ParsingType::kSpeechbank) {
            return parseSpeechResponse(response, value, symbols, localSymbols, functionTable);
        }
        return parseSimpleResponse(response, value);
    }
    if (type == g_RESPONSE_EVENT) {
        if (parsingType == DatabaseParser::ParsingType::kSimple) {
            return {JsonParseReturnCode::kInvalidValue, "Simple groups should not contain event responses"};
        }
        return parseEventResponse(response, value);
    }
    if (type == g_RESPONSE_CONTEXT) {
        if (parsingType == DatabaseParser::ParsingType::kSimple) {
            return {JsonParseReturnCode::kInvalidValue, "Simple groups should not contain context responses"};
        }
        return parseContextResponse(response, stringTable, value);
    }

    return {JsonParseReturnCode::kInvalidValue, "Unknown response type \"" + g_KEY_RESPONSE_TYPE + "\""};
}

JsonParseResult parseResponse(std::shared_ptr<Response>& response, StringTable& stringTable,
                              const rapidjson::Value& root, const DatabaseParser::ParsingType parsingType,
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
            return parseResponseObject(response, stringTable, value[0], parsingType, symbols, localSymbols,
                                       functionTable);
        } else {
            std::vector<std::shared_ptr<Response>> responses;
            for (auto iter = value.Begin(); iter != value.End(); ++iter) {
                std::shared_ptr<Response> item;
                auto result =
                    parseResponseObject(item, stringTable, *iter, parsingType, symbols, localSymbols, functionTable);
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

JsonParseResult parseRule(StringTable& stringTable, std::shared_ptr<RuleEntry>& rule,
                          std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId,
                          const rapidjson::Value& root, const std::string& idPrefix,
                          const DatabaseParser::ParsingType parsingType,
                          const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                          const std::unique_ptr<FunctionTable>& functionTable) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Rule must be a JSON object"};
    }

    int priority = 0;
    std::vector<std::shared_ptr<Criteria>> criteria;
    auto result = CriteriaParser::parseCriteria(criteria, priority, stringTable, root, namedRules);
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
    result = parseResponse(response, stringTable, root, parsingType, symbols, localSymbols, functionTable);
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

    rule = std::make_shared<RuleEntry>();
    rule->id = id;
    rule->criteria = criteria;
    rule->priority = priority;
    rule->response = response;
    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace Contextual::RuleParser