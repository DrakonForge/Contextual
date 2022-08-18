#include "ContextParser.h"

#include <memory>
#include <unordered_map>

#define RAPIDJSON_HAS_STDSTRING 1
#include <plog/Log.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "ContextManager.h"
#include "ContextTable.h"
#include "JsonUtils.h"

namespace Contextual::App {

namespace {

const std::string g_KEY_CONTEXT = "Context";

JsonParseResult parseContextTable(std::shared_ptr<ContextTable>& contextTable,
                                  const std::shared_ptr<ContextManager>& contextManager, const rapidjson::Value& root) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Context table value should be an object"};
    }
    contextTable = std::make_shared<ContextTable>(contextManager);
    for (auto iter = root.MemberBegin(); iter != root.MemberEnd(); ++iter) {
        const std::string& key = iter->name.GetString();
        if (iter->value.IsString()) {
            contextTable->set(key, iter->value.GetString());
        } else if (iter->value.IsNumber()) {
            contextTable->set(key, iter->value.GetFloat());
        } else if (iter->value.IsBool()) {
            contextTable->set(key, iter->value.GetBool());
        } else if (iter->value.IsArray()) {
            std::unordered_set<int> list;
            bool isStringList = true;
            for (auto iter2 = iter->value.GetArray().Begin(); iter2 != iter->value.GetArray().End(); ++iter2) {
                if (iter2->IsString()) {
                    list.insert(contextManager->getStringTable().cache(iter2->GetString()));
                } else if (iter2->IsNumber()) {
                    isStringList = false;
                    list.insert(iter2->GetInt());
                } else {
                    return {JsonParseReturnCode::kInvalidType, "Context list items can only be strings or integers"};
                }
            }
            contextTable->set(key, std::make_unique<std::unordered_set<int>>(std::move(list)), isStringList);
        } else {
            return {JsonParseReturnCode::kInvalidType, "Context value can only be string, number, bool, or array"};
        }
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

}  // namespace

namespace ContextParser {

JsonParseResult parseContextTables(std::unordered_map<std::string, std::shared_ptr<ContextTable>>& contextTables,
                                   const std::shared_ptr<ContextManager>& contextManager, const std::string& path) {
    rapidjson::Document document;
    bool fileExists = JsonUtils::readFile(path, document);
    if (!fileExists) {
        return {JsonParseReturnCode::kInvalidPath, "No file found at path \"" + path + "\""};
    }
    if (document.HasParseError()) {
        size_t offset = document.GetErrorOffset();
        const std::string& errorMsg = GetParseError_En(document.GetParseError());
        return {JsonParseReturnCode::kInvalidSyntax,
                "Invalid JSON at offset " + std::to_string(offset) + ": " + errorMsg};
    }

    if (document.HasMember(g_KEY_CONTEXT)) {
        const auto& value = document[g_KEY_CONTEXT];
        if (!value.IsObject()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CONTEXT + "\" should be an object"};
        }
        for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter) {
            const std::string& tableName = iter->name.GetString();
            std::shared_ptr<ContextTable> contextTable;
            auto result = parseContextTable(contextTable, contextManager, iter->value);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
            contextTables.emplace(tableName, contextTable);
        }
        return JsonUtils::g_RESULT_SUCCESS;
    }
    return {JsonParseReturnCode::kMissingKey, "Expected key \"" + g_KEY_CONTEXT + "\""};
}

}  // namespace ContextParser

}  // namespace Contextual::App