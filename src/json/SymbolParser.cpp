#include "SymbolParser.h"

#include <vector>

#include "JsonUtils.h"
#include "token/TokenBoolean.h"
#include "token/TokenContext.h"
#include "token/TokenFloat.h"
#include "token/TokenFunction.h"
#include "token/TokenInt.h"
#include "token/TokenList.h"
#include "token/TokenString.h"

namespace Contextual::SymbolParser {

namespace {

const std::string g_KEY_TYPE = "Type";
const std::string g_KEY_VALUE = "Value";
const std::string g_KEY_CONTEXT_TABLE = "Table";
const std::string g_KEY_CONTEXT_KEY = "Key";
const std::string g_KEY_FUNCTION_NAME = "Name";
const std::string g_KEY_FUNCTION_ARGS = "Args";
const std::string g_KEY_SYMBOLS = "Symbols";
const std::string g_KEY_SYMBOL_NAME = "Name";

const std::string g_TYPE_BOOLEAN = "Boolean";
const std::string g_TYPE_FLOAT = "Float";
const std::string g_TYPE_INTEGER = "Integer";
const std::string g_TYPE_STRING = "String";
const std::string g_TYPE_CONTEXT = "Context";
const std::string g_TYPE_FUNCTION = "Function";
const std::string g_TYPE_LIST = "List";
const std::string g_TYPE_SYMBOL = "Symbol";

// Forward declare this so it can be used recursively
JsonParseResult parseToken(std::shared_ptr<Token>& token, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols);

JsonParseResult parseBoolToken(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsBool()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_BOOLEAN + "\" must have a boolean value" };
    }
    token = std::make_shared<TokenBoolean>(value.GetBool());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFloatToken(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_FLOAT + "\" must have a numerical value" };
    }
    token = std::make_shared<TokenFloat>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseIntToken(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_INTEGER + "\" must have a numerical value" };
    }
    token = std::make_shared<TokenInt>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseStringToken(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
    if(!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_STRING + "\" must have a string value" };
    }
    token = std::make_shared<TokenString>(value.GetString());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseContextToken(std::shared_ptr<Token>& token, const rapidjson::Value& value) {
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

JsonParseResult parseFunctionToken(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
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
    if(!argValue.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_FUNCTION_ARGS + "\" must be an array"};
    }
    std::vector<std::shared_ptr<Token>> args;
    args.reserve(argValue.Size());
    for(auto iter = argValue.Begin(); iter != argValue.End(); ++iter) {
        std::shared_ptr<Token> arg;
        result = parseToken(arg, *iter, symbols);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        args.push_back(arg);
    }
    token = std::make_shared<TokenFunction>(name, std::move(args));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseListToken(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if(!value.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_LIST + "\" must have an array value" };
    }
    std::vector<std::shared_ptr<Token>> items;
    items.reserve(value.Size());
    for(auto iter = value.Begin(); iter != value.End(); ++iter) {
        std::shared_ptr<Token> item;
        auto result = parseToken(item, *iter, symbols);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        items.push_back(item);
    }
    token = std::make_shared<TokenList>(std::move(items));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSymbolToken(std::shared_ptr<Token>& token, const rapidjson::Value& value, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if(!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType, "Token of type \"" + g_TYPE_SYMBOL + "\" must have a string value" };
    }
    const std::string& name = value.GetString();
    auto got = symbols.find(name);
    if(got == symbols.end()) {
        return {JsonParseReturnCode::kMissingSymbol, "Symbol not defined: \"" + name + "\"" };
    }
    token = got->second;
    return JsonUtils::g_RESULT_SUCCESS;
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
        result = parseBoolToken(token, value);
    } else if(type == g_TYPE_FLOAT) {
        result = parseFloatToken(token, value);
    } else if(type == g_TYPE_INTEGER) {
        result = parseIntToken(token, value);
    } else if(type == g_TYPE_STRING) {
        result = parseStringToken(token, value);
    } else if(type == g_TYPE_CONTEXT) {
        result = parseContextToken(token, value);
    } else if(type == g_TYPE_FUNCTION) {
        result = parseFunctionToken(token, value, symbols);
    } else if(type == g_TYPE_LIST) {
        result = parseListToken(token, value, symbols);
    } else if(type == g_TYPE_SYMBOL) {
        result = parseSymbolToken(token, value, symbols);
    } else {
        return {JsonParseReturnCode::kInvalidValue, "Invalid token type \"" + type + "\""};
    }
    return result;
}

JsonParseResult parseSymbol(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Symbol must be a JSON object" };
    }
    if(!root.HasMember(g_KEY_SYMBOL_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Symbol must specify key \"" + g_KEY_SYMBOL_NAME + "\"" };
    }

    std::string name;
    auto result = JsonUtils::getString(name, root, g_KEY_SYMBOL_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    if(symbols.find(name) != symbols.end()) {
        return {JsonParseReturnCode::kAlreadyDefined, "Symbol \"" + name + "\" is already defined" };
    }

    std::shared_ptr<Token> token;
    result = parseToken(token, root, symbols);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    symbols.insert({name, token});
    return JsonUtils::g_RESULT_SUCCESS;
}

}

JsonParseResult parseSymbols(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root) {
    if(root.HasMember(g_KEY_SYMBOLS)) {
        const auto& value = root[g_KEY_SYMBOLS];
        if(!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_SYMBOLS + "\" must be an array" };
        }
        for(auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseSymbol(symbols, *iter);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }

    return JsonUtils::g_RESULT_SUCCESS;
}

}
