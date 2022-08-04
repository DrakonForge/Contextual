#include "TokenParser.h"

#include <vector>

#include "JsonUtils.h"
#include "TokenBoolean.h"
#include "TokenContext.h"
#include "TokenFloat.h"
#include "TokenFunction.h"
#include "TokenInt.h"
#include "TokenList.h"
#include "TokenString.h"

namespace Contextual::TokenParser {

namespace {

const std::string g_KEY_TYPE = "Type";
const std::string g_KEY_VALUE = "Type";

const std::string g_KEY_CONTEXT_TABLE = "Table";
const std::string g_KEY_CONTEXT_KEY = "Key";
const std::string g_KEY_FUNCTION_NAME = "Name";
const std::string g_KEY_FUNCTION_ARGS = "Args";

const std::string g_TYPE_BOOLEAN = "Boolean";
const std::string g_TYPE_FLOAT = "Float";
const std::string g_TYPE_INTEGER = "Integer";
const std::string g_TYPE_STRING = "String";
const std::string g_TYPE_CONTEXT = "Context";
const std::string g_TYPE_FUNCTION = "Function";
const std::string g_TYPE_LIST = "List";
const std::string g_TYPE_SYMBOL = "Symbol";

JsonParseResult parseBool(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsBool()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_BOOLEAN + "\" must have a boolean value" };
    }
    token = std::make_shared<TokenBoolean>(value.GetBool());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFloat(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_FLOAT + "\" must have a numerical value" };
    }
    token = std::make_shared<TokenFloat>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseInt(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_INTEGER + "\" must have a numerical value" };
    }
    token = std::make_shared<TokenInt>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseString(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_STRING + "\" must have a string value" };
    }
    token = std::make_shared<TokenString>(value.GetString());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseContext(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_CONTEXT + "\" must have an object value" };
    }
    if(!value.HasMember(g_KEY_CONTEXT_TABLE)) {
        return {JsonParseReturnCode::kMissingKey, "Context token must specify key \"" + g_KEY_CONTEXT_TABLE + "\"" };
    }
    if(!value.HasMember(g_KEY_CONTEXT_KEY)) {
        return {JsonParseReturnCode::kMissingKey, "Context token must specify key \"" + g_KEY_CONTEXT_KEY + "\"" };
    }
    std::string table;
    auto result = JsonUtils::getString(table, value, g_KEY_CONTEXT_TABLE);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::string key;
    result = JsonUtils::getString(key, value, g_KEY_CONTEXT_KEY);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    token = std::make_shared<TokenContext>(table, key);
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFunction(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if(!value.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_FUNCTION + "\" must have an object value" };
    }
    if(!value.HasMember(g_KEY_FUNCTION_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Function token must specify key \"" + g_KEY_FUNCTION_NAME + "\"" };
    }
    if(!value.HasMember(g_KEY_FUNCTION_ARGS)) {
        return {JsonParseReturnCode::kMissingKey, "Function token must specify key \"" + g_KEY_FUNCTION_ARGS + "\"" };
    }
    std::string name;
    auto result = JsonUtils::getString(name, value, g_KEY_FUNCTION_NAME);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    const auto& argValue = value[g_KEY_FUNCTION_ARGS];
    std::vector<std::shared_ptr<Token>> args;
    for(auto iter = argValue.Begin(); iter != argValue.End(); ++iter) {
        std::shared_ptr<Token> arg;
        result = parseToken(arg, *iter, symbols);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        args.push_back(arg);
    }
    token = std::make_shared<TokenFunction>(name, args);
}

JsonParseResult parseList(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if(!value.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_LIST + "\" must have an array value" };
    }
    std::vector<std::shared_ptr<Token>> items;
    for(auto iter = value.Begin(); iter != value.End(); ++iter) {
        std::shared_ptr<Token> item;
        auto result = parseToken(item, *iter, symbols);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        items.push_back(item);
    }
    token = std::make_shared<TokenList>(items);
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSymbol(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if(!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_SYMBOL + "\" must have a string value" };
    }
    const std::string& name = value.GetString();
    if(symbols.find(name) == symbols.end()) {
        return {JsonParseReturnCode::kMissingSymbol, "Symbol not defined: \"" + name + "\"" };
    }
    // TODO: Not sure if this needs to be wrapped in an std::shared_ptr for copying or not
    token = symbols.at(name);
    return JsonUtils::g_RESULT_SUCCESS;
}

}

JsonParseResult parseToken(std::shared_ptr<Token>& token, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Token must be a JSON object" };
    }
    if(!root.HasMember(g_KEY_TYPE)) {
        return { JsonParseReturnCode::kMissingKey, "Token must specify key \"" + g_KEY_TYPE + "\""};
    }
    if(!root.HasMember(g_KEY_VALUE)) {
        return { JsonParseReturnCode::kMissingKey, "Token must specify key \"" + g_KEY_VALUE + "\""};
    }
    std::string type;
    auto result = JsonUtils::getString(type, root, g_KEY_TYPE);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    const auto& value = root[g_KEY_VALUE];

    if(type == g_TYPE_BOOLEAN) {
        result = parseBool(token, value);
    } else if(type == g_TYPE_FLOAT) {
        result = parseFloat(token, value);
    } else if(type == g_TYPE_INTEGER) {
        result = parseInt(token, value);
    } else if(type == g_TYPE_STRING) {
        result = parseString(token, value);
    } else if(type == g_TYPE_CONTEXT) {
        result = parseContext(token, value);
    } else if(type == g_TYPE_FUNCTION) {
        result = parseFunction(token, value, symbols);
    } else if(type == g_TYPE_LIST) {
        result = parseList(token, value, symbols);
    } else if(type == g_TYPE_SYMBOL) {
        result = parseSymbol(token, value, symbols);
    } else {
        return {JsonParseReturnCode::kInvalidValue, "Invalid token type \"" + type + "\""};
    }
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

}
