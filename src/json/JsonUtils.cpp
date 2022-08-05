#include "JsonUtils.h"

namespace Contextual::JsonUtils {

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