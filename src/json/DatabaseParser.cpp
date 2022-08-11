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
#include "SymbolParser.h"
#include "token/SymbolToken.h"

namespace Contextual::RuleParser {

namespace {

// Constants

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_PARENT = "Parent";
const std::string g_KEY_TYPE = "Type";
const std::string g_KEY_CATEGORIES = "Categories";
const std::string g_KEY_CATEGORY_NAME = "Name";
const std::string g_KEY_CATEGORY_RULES = "Rules";

const std::string g_TYPE_DEFAULT = "Default";
const std::string g_TYPE_SIMPLE = "Simple";
const std::string g_TYPE_SPEECHBANK = "Speechbank";

const std::string g_EXT_JSON = ".json";

// Structs

struct ParsedGroup {
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>> symbols;
    std::unordered_map<std::string, RuleParser::RuleInfo> namedRules;
};

struct QueuedGroup {
    rapidjson::Value& value;
    std::string name;
    std::string path;
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
        } else if (typeStr == g_TYPE_SIMPLE) {
            type = ParsingType::kSimple;
        } else {
            return {JsonParseReturnCode::kInvalidValue, "Type \"" + typeStr + "\" not supported"};
        }
    } else {
        type = ParsingType::kDefault;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseCategory(ParsedData& parsedData, std::unordered_map<std::string, RuleParser::RuleInfo>& namedRules,
                              const ParsingType parsingType, const rapidjson::Value& root,
                              const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                              const std::string& groupName) {
    if (!root.IsObject()) {
        return {JsonParseReturnCode::kInvalidType, "Category must be a JSON object"};
    }
    if (!root.HasMember(g_KEY_CATEGORY_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Category must specify key \"" + g_KEY_CATEGORY_NAME + "\""};
    }
    if (!root.HasMember(g_KEY_CATEGORY_RULES)) {
        return {JsonParseReturnCode::kMissingKey, "Category must specify key \"" + g_KEY_CATEGORY_RULES + "\""};
    }

    // Get category name
    std::string categoryName;
    auto result = JsonUtils::getString(categoryName, root, g_KEY_CATEGORY_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }
    if (parsedData.database.getRuleTable(groupName, categoryName) != nullptr) {
        return {JsonParseReturnCode::kAlreadyDefined,
                "Category \"" + categoryName + "\" for group \"" + groupName + "\" is already defined"};
    }

    const auto& rulesValue = root[g_KEY_CATEGORY_RULES];
    if (!rulesValue.IsArray()) {
        return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CATEGORY_RULES + "\" must be an array"};
    }

    auto ruleTable = std::make_unique<RuleTable>();
    int nextId = 0;  // ID used for unnamed rules
    const std::string idPrefix = groupName + "." + categoryName + ".";
    StringTable& stringTable = parsedData.database.getContextManager().getStringTable();
    for (auto iter = rulesValue.Begin(); iter != rulesValue.End(); ++iter) {
        std::unique_ptr<RuleEntry> ruleEntry;
        result = RuleParser::parseRule(stringTable, ruleEntry, namedRules, nextId, *iter, idPrefix, parsingType,
                                       symbols, parsedData.database.getContextManager().getFunctionTable());
        if (result.code == JsonParseReturnCode::kSkipCreation) {
            continue;
        }
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        ruleTable->addEntry(ruleEntry);
        ++parsedData.stats.numRules;
    }
    if (ruleTable->getNumEntries() > 0) {
        ruleTable->sortEntries();
        parsedData.database.addRuleTable(groupName, categoryName, ruleTable);
        ++parsedData.stats.numTables;
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseCategories(ParsedData& parsedData,
                                std::unordered_map<std::string, RuleParser::RuleInfo>& namedRules,
                                const ParsingType parsingType, const rapidjson::Value& root,
                                const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                const std::string& groupName) {
    if (root.HasMember(g_KEY_CATEGORIES)) {
        const auto& value = root[g_KEY_CATEGORIES];
        if (!value.IsArray()) {
            return {JsonParseReturnCode::kInvalidType, "Key \"" + g_KEY_CATEGORIES + "\" must be an array"};
        }
        for (auto iter = value.Begin(); iter != value.End(); ++iter) {
            auto result = parseCategory(parsedData, namedRules, parsingType, *iter, symbols, groupName);
            if (result.code != JsonParseReturnCode::kSuccess) {
                return result;
            }
        }
    }
    return JsonUtils::g_RESULT_SUCCESS;
}

JsonParseResult parseGroup(ParsedData& parsedData, const rapidjson::Value& root, const std::string& name,
                           const std::optional<ParsedGroup>& parsedParent) {
    // Type
    ParsingType parsingType;
    auto result = getParsingType(parsingType, root);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Symbols
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>> symbols;
    // Simple groups do not use symbols
    if (parsingType != ParsingType::kSimple) {
        // Copy over parent symbols
        if (parsedParent) {
            symbols.insert(parsedParent->symbols.begin(), parsedParent->symbols.end());
        }
        result = SymbolParser::parseSymbols(symbols, root, parsedData.database.getContextManager().getFunctionTable());
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
    }

    std::unordered_map<std::string, RuleParser::RuleInfo> namedRules;
    result = parseCategories(parsedData, namedRules, parsingType, root, symbols, name);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    parsedData.parsedGroups.insert({name, {symbols, namedRules}});
    return JsonUtils::g_RESULT_SUCCESS;
}

// Main methods

JsonParseResult readGroup(ParsedData& parsedData, const std::string& path, bool allowMissingParent) {
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

    // Name
    if (!document.HasMember(g_KEY_NAME)) {
        return {JsonParseReturnCode::kMissingKey, "Group must specify key \"" + g_KEY_NAME + "\""};
    }
    std::string name;
    auto result = JsonUtils::getString(name, document, g_KEY_NAME);
    if (result.code != JsonParseReturnCode::kSuccess) {
        return result;
    }

    // Parent
    std::optional<ParsedGroup> parsedParent;
    if (document.HasMember(g_KEY_PARENT)) {
        std::string parentName;
        result = JsonUtils::getString(parentName, document, g_KEY_PARENT);
        if (result.code != JsonParseReturnCode::kSuccess) {
            return result;
        }
        auto got = parsedData.parsedGroups.find(parentName);
        if (got == parsedData.parsedGroups.end()) {
            if (!allowMissingParent) {
                // Parent is not loaded yet, add to queue
                parsedData.queuedGroups.push({document, name, path, parentName});
                return {JsonParseReturnCode::kSkipCreation, ""};
            }
        } else {
            parsedParent = got->second;
        }
    }

    return parseGroup(parsedData, document, name, parsedParent);
}

void resolveQueuedGroups(ParsedData& parsedData) {
    // There's probably a less brute-force way to do this
    // But do I look that fancy?
    auto& queue = parsedData.queuedGroups;
    size_t checkAfter = queue.size();
    bool changed = false;

    while (!queue.empty()) {
        const auto& item = queue.front();

        auto got = parsedData.parsedGroups.find(item.parentName);
        if (got == parsedData.parsedGroups.end()) {
            // Parent still not loaded, add back to queue
            queue.push(item);
        } else {
            // Parent now loaded, proceed with parsing group
            auto result = parseGroup(parsedData, item.value, item.name, got->second);
            if (result.code == JsonParseReturnCode::kSuccess) {
                std::cout << "Successfully parsed " << item.path << "\n";
                ++parsedData.stats.numLoaded;
            } else if (result.code != JsonParseReturnCode::kSkipCreation) {
                std::cerr << "Failed to parse " << item.path << ": " << result.errorMsg << "\n";
                ++parsedData.stats.numFailed;
            }
            changed = true;
        }
        queue.pop();  // Make sure to pop only AFTER we're done using the item

        // After going through the entire queue once, check if any groups were
        // successfully loaded. If not, then all that's left are missing/circular
        // dependencies.
        if (--checkAfter == 0) {
            if (!changed) {
                // Gather list of all failed groups
                parsedData.stats.numFailed += queue.size();
                while (!queue.empty()) {
                    const auto& failedItem = queue.front();
                    std::cerr << "Failed to parse " << failedItem.path
                              << ": Unknown parent \"" + failedItem.parentName +
                                     "\" (is it missing or circular reference)?\n";
                    queue.pop();
                }
            }
            // Reset counters--probably leads to some repetitions if there are multiple successes in a loop but oh well
            checkAfter = queue.size();
            changed = false;
        }
    }
}

void readAllFiles(ParsedData& parsedData, const std::string& dirPath) {
    if (!std::filesystem::is_directory(dirPath)) {
        std::cerr << "No directory found at path \"" << dirPath << "\"\n";
        return;
    }

    const auto& dir = std::filesystem::recursive_directory_iterator(dirPath);
    for (const auto& file : dir) {
        const std::string& extension = file.path().extension().string();
        if (extension == g_EXT_JSON) {
            const std::string& path = file.path().string();
            auto result = readGroup(parsedData, path, false);
            if (result.code == JsonParseReturnCode::kSuccess) {
                std::cout << "Successfully parsed " << path << "\n";
                ++parsedData.stats.numLoaded;
            } else if (result.code != JsonParseReturnCode::kSkipCreation) {
                std::cerr << "Failed to parse " << path << ": " << result.errorMsg << "\n";
                ++parsedData.stats.numFailed;
            }
        }
    }
    resolveQueuedGroups(parsedData);
}

}  // namespace

JsonParseResult loadGroup(RuleDatabase& database, const std::string& path) {
    ParsedData parsedData{database};
    return readGroup(parsedData, path, true);
}

DatabaseStats loadDatabase(RuleDatabase& database, const std::string& dirPath) {
    ParsedData parsedData{database};
    readAllFiles(parsedData, dirPath);
    return parsedData.stats;
}

}  // namespace Contextual::RuleParser