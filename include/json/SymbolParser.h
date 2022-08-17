#pragma once

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#include <memory>

#include "JsonUtils.h"
#include "SymbolToken.h"

namespace Contextual::SymbolParser {

JsonParseResult parseSymbols(
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols, const rapidjson::Value& root,
    const std::optional<std::unordered_map<std::string, std::shared_ptr<SymbolToken>>>& otherSymbols,
    const std::unique_ptr<FunctionTable>& functionTable);
std::string tokenTypeToString(TokenType type);

}  // namespace Contextual::SymbolParser