#pragma once

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#include <memory>

#include "JsonUtils.h"
#include "token/Token.h"

namespace Contextual::SymbolParser {

JsonParseResult parseSymbols(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root);

}