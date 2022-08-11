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
const std::string g_TYPE_UNKNOWN = "Unknown";

// Forward declare this so it can be used recursively
JsonParseResult parseToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& root,
                           const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                           const FunctionTable& functionTable);

JsonParseResult parseBoolToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value) {
    if (!value.IsBool()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_BOOLEAN + "\" must have a boolean value"};
    }
    token = std::make_shared<TokenBoolean>(value.GetBool());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFloatToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value) {
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_FLOAT + "\" must have a numerical value"};
    }
    token = std::make_shared<TokenFloat>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseIntToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value) {
    if (!value.IsNumber()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_INTEGER + "\" must have a numerical value"};
    }
    token = std::make_shared<TokenInt>(value.GetFloat());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseStringToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value) {
    if (!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_STRING + "\" must have a string value"};
    }
    token = std::make_shared<TokenString>(value.GetString());
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseContextToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value) {
    if (!value.IsObject()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_CONTEXT + "\" must have an object value"};
    }
    if (!value.HasMember(g_KEY_CONTEXT_TABLE)) {
        return {JsonParseReturnCode::kMissingKey, "Context token must specify key \"" + g_KEY_CONTEXT_TABLE + "\""};
    }
    if (!value.HasMember(g_KEY_CONTEXT_KEY)) {
        return {JsonParseReturnCode::kMissingKey, "Context token must specify key \"" + g_KEY_CONTEXT_KEY + "\""};
    }
    std::string table;
    auto result = JsonUtils::getString(table, value, g_KEY_CONTEXT_TABLE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::string key;
    result = JsonUtils::getString(key, value, g_KEY_CONTEXT_KEY);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    token = std::make_shared<TokenContext>(table, key);
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseFunctionToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value,
                                   const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                   const FunctionTable& functionTable) {
    if (!value.IsObject()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_FUNCTION + "\" must have an object value"};
    }
    if (!value.HasMember(g_KEY_FUNCTION_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Function token must specify key \"" + g_KEY_FUNCTION_NAME + "\""};
    }
    if (!value.HasMember(g_KEY_FUNCTION_ARGS)) {
        return {JsonParseReturnCode::kMissingKey, "Function token must specify key \"" + g_KEY_FUNCTION_ARGS + "\""};
    }
    std::string name;
    auto result = JsonUtils::getString(name, value, g_KEY_FUNCTION_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    const auto& argValue = value[g_KEY_FUNCTION_ARGS];
    if (!argValue.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_FUNCTION_ARGS + "\" must be an array"};
    }
    std::vector<std::shared_ptr<SymbolToken>> args;
    args.reserve(argValue.Size());
    for (auto iter = argValue.Begin(); iter != argValue.End(); ++iter) {
        std::shared_ptr<SymbolToken> arg;
        result = parseToken(arg, *iter, symbols, functionTable);
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        args.push_back(arg);
    }

    // Check that function exists
    const auto& sig = functionTable.getSignature(name);
    if (sig == nullptr) {
        return {JsonParseReturnCode::kInvalidFunction, "Function \"" + name + "\" does not exist"};
    }

    // Check that args are valid
    if (!FunctionTable::validateArgs(sig, args)) {
        std::string argList;
        if (sig->argTypes.empty()) {
            argList = "Expected no arguments";
        } else {
            std::string varArgsStr;
            if (sig->hasVarArgs) {
                varArgsStr = "at least ";
            }
            argList = "Expected " + varArgsStr + std::to_string(sig->argTypes.size()) + " arguments of type " +
                      tokenTypeToString(sig->argTypes[0]);
            for (int i = 1; i < sig->argTypes.size(); ++i) {
                argList += ", " + tokenTypeToString(sig->argTypes[i]);
            }
        }
        return {JsonParseReturnCode::kInvalidFunction, "Invalid arguments for function \"" + name + "\": " + argList};
    }

    token = std::make_shared<TokenFunction>(std::move(name), std::move(args));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseListToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value,
                               const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols, const FunctionTable& functionTable) {
    if (!value.IsArray()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_LIST + "\" must have an array value"};
    }
    if (value.Empty()) {
        return {JsonParseReturnCode::kInvalidValue, "List token cannot be empty"};
    }
    std::vector<std::shared_ptr<SymbolToken>> items;
    items.reserve(value.Size());
    for (auto iter = value.Begin(); iter != value.End(); ++iter) {
        std::shared_ptr<SymbolToken> item;
        auto result = parseToken(item, *iter, symbols, functionTable);
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        items.push_back(item);
    }
    token = std::make_shared<TokenList>(std::move(items));
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSymbolToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& value,
                                 const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols) {
    if (!value.IsString()) {
        return {JsonParseReturnCode::kInvalidType,
                "SymbolToken of type \"" + g_TYPE_SYMBOL + "\" must have a string value"};
    }
    const std::string& name = value.GetString();
    auto got = symbols.find(name);
    if (got == symbols.end()) {
        return {JsonParseReturnCode::kMissingSymbol, "Symbol not defined: \"" + name + "\""};
    }
    token = got->second;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseToken(std::shared_ptr<SymbolToken>& token, const rapidjson::Value& root,
                           const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                           const FunctionTable& functionTable) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "SymbolToken must be a JSON object"};
    }
    if (!root.HasMember(g_KEY_TYPE)) {
        return {JsonParseReturnCode::kMissingKey, "SymbolToken must specify key \"" + g_KEY_TYPE + "\""};
    }
    if (!root.HasMember(g_KEY_VALUE)) {
        return {JsonParseReturnCode::kMissingKey, "SymbolToken must specify key \"" + g_KEY_VALUE + "\""};
    }
    std::string type;
    auto result = JsonUtils::getString(type, root, g_KEY_TYPE);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    const auto& value = root[g_KEY_VALUE];

    if (type == g_TYPE_BOOLEAN) {
        result = parseBoolToken(token, value);
    } else if (type == g_TYPE_FLOAT) {
        result = parseFloatToken(token, value);
    } else if (type == g_TYPE_INTEGER) {
        result = parseIntToken(token, value);
    } else if (type == g_TYPE_STRING) {
        result = parseStringToken(token, value);
    } else if (type == g_TYPE_CONTEXT) {
        result = parseContextToken(token, value);
    } else if (type == g_TYPE_FUNCTION) {
        result = parseFunctionToken(token, value, symbols, functionTable);
    } else if (type == g_TYPE_LIST) {
        result = parseListToken(token, value, symbols, functionTable);
    } else if (type == g_TYPE_SYMBOL) {
        result = parseSymbolToken(token, value, symbols);
    } else {
        return {JsonParseReturnCode::kInvalidValue, "Invalid token type \"" + type + "\""};
    }
    return result;
}

JsonParseResult parseSymbol(std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                            const rapidjson::Value& root, const FunctionTable& functionTable) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Symbol must be a JSON object"};
    }
    if (!root.HasMember(g_KEY_SYMBOL_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Symbol must specify key \"" + g_KEY_SYMBOL_NAME + "\""};
    }

    std::string name;
    auto result = JsonUtils::getString(name, root, g_KEY_SYMBOL_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    if (symbols.find(name) != symbols.end()) {
        return {JsonParseReturnCode::kAlreadyDefined, "Symbol \"" + name + "\" is already defined"};
    }

    std::shared_ptr<SymbolToken> token;
    result = parseToken(token, root, symbols, functionTable);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    symbols.emplace(name, token);
    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace

std::string tokenTypeToString(const TokenType type) {
    if (type == TokenType::kString) {
        return g_TYPE_STRING;
    }
    if (type == TokenType::kContext) {
        return g_TYPE_CONTEXT;
    }
    if (type == TokenType::kBool) {
        return g_TYPE_BOOLEAN;
    }
    if (type == TokenType::kFloat) {
        return g_TYPE_FLOAT;
    }
    if (type == TokenType::kInt) {
        return g_TYPE_INTEGER;
    }
    if (type == TokenType::kFunction) {
        return g_TYPE_FUNCTION;
    }
    if (type == TokenType::kList) {
        return g_TYPE_LIST;
    }
    return g_TYPE_UNKNOWN;
}

JsonParseResult parseSymbols(std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                             const rapidjson::Value& root, const FunctionTable& functionTable) {
    if (root.HasMember(g_KEY_SYMBOLS)) {
        const auto& value = root[g_KEY_SYMBOLS];
        if (!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_SYMBOLS + "\" must be an array"};
        }
        for (auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseSymbol(symbols, *iter, functionTable);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }

    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace Contextual::SymbolParser
