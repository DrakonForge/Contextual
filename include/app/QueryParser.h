#pragma once

#include "AppSettings.h"
#include "JsonUtils.h"

namespace Contextual::App::QueryParser {

JsonParseResult parseQuery(AppSettings& settings, const std::string& queryPath);

}