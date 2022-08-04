#pragma once

#include <fstream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace Contextual {

enum class JsonParseReturnCode : uint32_t {
    kSuccess = 0,
    kParentNotYetLoaded = 1,
    kInvalidPath = 10,
    kInvalidSyntax = 11,
    kInvalidType = 12,
    kInvalidValue = 13,
    kMissingKey = 20,
    kMissingSymbol = 21,
    kAlreadyDefined = 30
};

struct JsonParseResult {
    JsonParseReturnCode code;
    std::string errorMsg;
};

namespace JsonUtils {

const JsonParseResult g_RESULT_SUCCESS = {JsonParseReturnCode::kSuccess, "" };

bool readFile(const std::string& path, rapidjson::Document& out) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        return false;
    }
    rapidjson::IStreamWrapper isw(ifs);
    out.ParseStream(isw);
    return true;
}

JsonParseResult getString(std::string& str, const rapidjson::Value& obj, const std::string& key) {
    const auto& value = obj[key];
    if (value.IsString()) {
        str = value.GetString();
        return JsonUtils::g_RESULT_SUCCESS;
    }
    return {JsonParseReturnCode::kInvalidType, "Key \"" + key + "\" must be a string" };
}

}

}
