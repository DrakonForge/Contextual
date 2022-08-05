#include "DatabaseParser.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <queue>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "JsonUtils.h"
#include "RuleParser.h"
#include "token/Token.h"
#include "TokenParser.h"

namespace Contextual::RuleParser {

namespace {

// Constants

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_PARENT = "Parent";
const std::string g_KEY_TYPE = "Type";
const std::string g_KEY_SYMBOLS = "Symbols";
const std::string g_KEY_CATEGORIES = "Categories";

const std::string g_KEY_SYMBOL_NAME = "Name";

const std::string g_KEY_CATEGORY_NAME = "Name";
const std::string g_KEY_CATEGORY_RULES = "Rules";

const std::string g_TYPE_DEFAULT = "Default";
const std::string g_TYPE_SPEECHBANK = "Speechbank";

const std::string g_EXT_JSON = ".json";

// Structs

struct ParsedGroup {
    std::unordered_map<std::string, std::shared_ptr<Token>> symbols;
    std::unordered_map<std::string, RuleParser::RuleInfo> namedRules;
};

struct QueuedGroup {
    rapidjson::Value& value;
    std::string name;
    std::string parentName;
};

struct ParsedData {
    RuleDatabase& database;
    std::queue<QueuedGroup> queuedGroups;
    std::unordered_map<std::string, ParsedGroup> parsedGroups;
    DatabaseStats stats = {0, 0, 0, 0};
};

// Helper methods

// Parsing methods

JsonParseResult getParsingType(ParsingType& type, const rapidjson::Value& root) {
    if (root.HasMember(g_KEY_TYPE)) {
        std::string typeStr;
        auto result = JsonUtils::getString(typeStr, root, g_KEY_TYPE);
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        if (typeStr == g_TYPE_DEFAULT) {
            type = ParsingType::kDefault;
        } else if (typeStr == g_TYPE_SPEECHBANK) {
            type = ParsingType::kSpeechbank;
        } else {
            return {JsonParseReturnCode::kInvalidValue, "Type \"" + typeStr + "\" not supported" };
        }
    } else {
        type = ParsingType::kDefault;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSymbol(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Symbol must be a JSON object" };
    }
    if(!root.HasMember(g_KEY_SYMBOL_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Symbol must specify key \"" + g_KEY_SYMBOL_NAME + "\"" };
    }

    std::string name;
    auto result = JsonUtils::getString(name, root, g_KEY_SYMBOL_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    if(symbols.find(name) != symbols.end()) {
        return {JsonParseReturnCode::kAlreadyDefined, "Symbol \"" + name + "\" is already defined" };
    }

    std::shared_ptr<Token> token;
    result = TokenParser::parseToken(token, root, symbols);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    symbols.insert({name, token});
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseSymbols(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root, const std::optional<ParsedGroup>& parsedParent) {
    if (parsedParent) {
        symbols.insert(parsedParent->symbols.begin(), parsedParent->symbols.end());
    }
    if(root.HasMember(g_KEY_SYMBOLS)) {
        const auto& value = root[g_KEY_SYMBOLS];
        if(!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_SYMBOLS + "\" must be an array" };
        }
        for(auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseSymbol(symbols, *iter);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }

    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseCategory(ParsedData& parsedData, std::unordered_map<std::string, RuleParser::RuleInfo>& namedRules, const ParsingType type, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const std::string& groupName) {
    if(!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Category must be a JSON object"};
    }
    if(!root.HasMember(g_KEY_CATEGORY_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Category must specify key \"" + g_KEY_CATEGORY_NAME + "\""};
    }
    if(!root.HasMember(g_KEY_CATEGORY_RULES)) {
        return {JsonParseReturnCode::kMissingKey, "Category must specify key \"" + g_KEY_CATEGORY_RULES + "\""};
    }
    std::string categoryName;
    auto result = JsonUtils::getString(categoryName, root, g_KEY_CATEGORY_NAME);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    if(parsedData.database.getRuleTable(groupName, categoryName) != nullptr) {
        return {JsonParseReturnCode::kAlreadyDefined, "Category \"" + categoryName + "\" for group \"" + groupName + "\" is already defined" };
    }

    const auto& rulesValue = root[g_KEY_CATEGORY_RULES];
    if(!rulesValue.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CATEGORY_RULES + "\" must be an array"};
    }

    auto ruleTable = std::make_unique<RuleTable>();
    int nextId = 0; // ID used for unnamed rules
    const std::string idPrefix = groupName + "." + categoryName + ".";
    StringTable& stringTable = parsedData.database.getContextManager().getStringTable();
    for(auto iter = rulesValue.Begin(); iter != rulesValue.End(); ++iter) {
        std::unique_ptr<RuleEntry> ruleEntry;
        result = RuleParser::parseRule(stringTable, ruleEntry, namedRules, nextId, *iter, idPrefix, type, symbols);
        if(result.code == JsonParseReturnCode::kSkipCreation) {
            continue;
        }
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        ruleTable->addEntry(ruleEntry);
        ++parsedData.stats.numRules;
    }
    ruleTable->sortEntries();
    parsedData.database.addRuleTable(groupName, categoryName, ruleTable);
    ++parsedData.stats.numTables;
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseCategories(ParsedData& parsedData, std::unordered_map<std::string, RuleParser::RuleInfo>& namedRules, const ParsingType type, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const std::string& groupName) {
    if (root.HasMember(g_KEY_CATEGORIES)) {
        const auto& value = root[g_KEY_CATEGORIES];
        if (!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CATEGORIES + "\" must be an array" };
        }
        for(auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseCategory(parsedData, namedRules, type, *iter, symbols, groupName);
        }
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseGroup(ParsedData& parsedData, const rapidjson::Value& root, const std::string& name, const std::optional<ParsedGroup>& parsedParent) {
    // Type
    ParsingType type;
    auto result = getParsingType(type, root);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Symbols
    std::unordered_map<std::string, std::shared_ptr<Token>> symbols;
    result = parseSymbols(symbols, root, parsedParent);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    std::unordered_map<std::string, RuleParser::RuleInfo> namedRules;
    result = parseCategories(parsedData, namedRules, type, root, symbols, name);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    parsedData.parsedGroups.insert({name, {symbols, namedRules}});
    return JsonUtils::g_RESULT_SUCCESS;
}

// Main methods

JsonParseResult readGroup(ParsedData& parsedData, const std::string& path, bool allowMissingParent) {
    rapidjson::Document document;
    bool fileExists = JsonUtils::readFile(path, document);
    if(!fileExists) {
        return {JsonParseReturnCode::kInvalidPath, "No file found at path \"" + path + "\"" };
    }
    if (document.HasParseError()) {
        size_t offset = document.GetErrorOffset();
        const std::string& errorMsg = GetParseError_En(document.GetParseError());
        return {JsonParseReturnCode::kInvalidSyntax, "Invalid JSON at offset " + std::to_string(offset) + ": " + errorMsg };
    }

    // Name
    if (!document.HasMember(g_KEY_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Group must specify key \"" + g_KEY_NAME + "\"" };
    }
    std::string name;
    auto result = JsonUtils::getString(name, document, g_KEY_NAME);
    if(result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Parent
    std::optional<ParsedGroup> parsedParent;
    if (document.HasMember(g_KEY_PARENT)) {
        std::string parentName;
        result = JsonUtils::getString(parentName, document, g_KEY_PARENT);
        if(result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        if(parsedData.parsedGroups.find(parentName) == parsedData.parsedGroups.end()) {
            if(!allowMissingParent) {
                // Parent is not loaded yet, add to queue
                parsedData.queuedGroups.push({document, name, parentName});
                return {JsonParseReturnCode::kSkipCreation, ""};
            }
        } else {
            parsedParent = parsedData.parsedGroups.at(parentName);
        }
    }

    return parseGroup(parsedData, document, name, parsedParent);
}

void readAllFiles(ParsedData& parsedData, const std::string& dirPath) {
    if(!std::filesystem::is_directory(dirPath)) {
        std::cerr << "No directory found at path \"" << dirPath << "\"\n";
        return;
    }

    const auto& dir = std::filesystem::recursive_directory_iterator(dirPath);
    for (const auto& file : dir) {
        const std::string& extension = file.path().extension().string();
        if(extension == g_EXT_JSON) {
            const std::string& pathStr = file.path().string();
            JsonParseResult result = readGroup(parsedData, pathStr, false);
            if(result.code == JsonParseReturnCode::kSuccess) {
                std::cout << "Successfully parsed " << pathStr << "\n";
                ++parsedData.stats.numLoaded;
            } else if(result.code != JsonParseReturnCode::kSkipCreation) {
                std::cerr << "Failed to parse " << pathStr << ": " << result.errorMsg << "\n";
                ++parsedData.stats.numFailed;
            }
        }
    }
}

void resolveQueuedGroups(ParsedData& parsedData) {
    if(parsedData.queuedGroups.empty()) {
        return;
    }

    // TODO
}

}

JsonParseResult loadGroup(RuleDatabase& database, const std::string& path) {
    ParsedData parsedData {database};
    return readGroup(parsedData, path, true);
}

DatabaseStats loadDatabase(RuleDatabase& database, const std::string& dirPath) {
    ParsedData parsedData {database};
    readAllFiles(parsedData, dirPath);
    resolveQueuedGroups(parsedData);
    return parsedData.stats;
}

}