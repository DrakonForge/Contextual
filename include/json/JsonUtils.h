#pragma once

#include <fstream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace Contextual {

enum class JsonParseReturnCode : uint32_t {
    kSuccess = 0,
    kSkipCreation = 1,
    kInvalidPath = 10,
    kInvalidSyntax = 11,
    kInvalidType = 12,
    kInvalidValue = 13,
    kMissingKey = 20,
    kMissingSymbol = 21,
    kMissingRule = 22,
    kAlreadyDefined = 30
};

struct JsonParseResult {
    JsonParseReturnCode code;
    std::string errorMsg;
};

namespace JsonUtils {

const JsonParseResult g_RESULT_SUCCESS = {JsonParseReturnCode::kSuccess, ""};
bool readFile(const std::string& path, rapidjson::Document& out);
JsonParseResult getString(std::string& str, const rapidjson::Value& obj, const std::string& key);

}  // namespace JsonUtils

}  // namespace Contextual
