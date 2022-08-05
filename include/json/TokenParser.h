#pragma once

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#include <memory>

#include "JsonUtils.h"
#include "token/Token.h"

namespace Contextual::TokenParser {

JsonParseResult parseToken(std::shared_ptr<Token>& token, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols);

}