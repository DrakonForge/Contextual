#include "RuleParser.h"

#include "CriteriaParser.h"
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

const std::string g_RESPONSE_RANDOM = "Random";
const std::string g_RESPONSE_EVENT = "Event";
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
    if (type == g_RESPONSE_EVENT) {
        if (parsingType == ParsingType::kSimple) {
            return {JsonParseReturnCode::kInvalidValue, "Simple groups should not contain event responses"};
        }
        return parseEventResponse(response, value);
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

JsonParseResult parseRule(StringTable& stringTable, std::shared_ptr<RuleEntry>& rule,
                          std::unordered_map<std::string, RuleInfo>& namedRules, int& nextId,
                          const rapidjson::Value& root, const std::string& idPrefix, const ParsingType parsingType,
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

    rule = std::make_shared<RuleEntry>();
    rule->id = id;
    rule->criteria = criteria;
    rule->priority = priority;
    rule->response = response;
    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace Contextual::RuleParser