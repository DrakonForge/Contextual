#pragma once

#include <memory>
#include <unordered_map>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#include "ContextManager.h"
#include "ContextTable.h"
#include "JsonUtils.h"

namespace Contextual::App::ContextParser {

JsonParseResult parseContextTables(std::unordered_map<std::string, std::shared_ptr<ContextTable>>& contextTables,
                                   const std::shared_ptr<ContextManager>& contextManager, const std::string& path);

}