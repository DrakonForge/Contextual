#include "RuleParser.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <optional>
#include <queue>

#include "Response.h"
#include "Token.h"

namespace Contextual::RuleParser {

namespace {

// Constants

const std::string g_KEY_NAME = "Name";
const std::string g_KEY_PARENT = "Parent";
const std::string g_KEY_TYPE = "Type";
const std::string g_KEY_SYMBOLS = "Symbols";
const std::string g_KEY_CATEGORIES = "Categories";

const std::string g_TYPE_DEFAULT = "Default";
const std::string g_TYPE_SPEECHBANK = "Speechbank";

const std::string g_EXT_JSON = ".json";

const RuleParserResult g_RESULT_SUCCESS = { RuleParserReturnCode::kSuccess, "" };

// Enums and Structs

enum class ParsingType : uint8_t {
    kDefault,
    kSpeechbank
};

struct RuleInfo {
    std::vector<Criteria> criteria;
    std::shared_ptr<Response> response;
};

struct ParsedGroup {
    std::unordered_map<std::string, std::shared_ptr<Token>> symbols;
    std::unordered_map<std::string, RuleInfo> namedRules;
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

bool readFile(const std::string& path, rapidjson::Document& out) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        return false;
    }
    rapidjson::IStreamWrapper isw(ifs);
    out.ParseStream(isw);
    return true;
}

RuleParserResult getString(std::string& str, const rapidjson::Value& obj, const std::string& key) {
    const auto& value = obj[key];
    if (value.IsString()) {
        str = value.GetString();
        return g_RESULT_SUCCESS;
    }
    return { RuleParserReturnCode::kInvalidType, "Key \"" + key + "\" must be a string" };
}

// Parsing methods

RuleParserResult getParsingType(ParsingType& type, const rapidjson::Value& root) {
    if (root.HasMember(g_KEY_TYPE)) {
        std::string typeStr;
        auto result = getString(typeStr, root, g_KEY_TYPE);
        if (result.code != RuleParserReturnCode::kSuccess) {
            return result;
        }
        if (typeStr == g_TYPE_DEFAULT) {
            type = ParsingType::kDefault;
        } else if (typeStr == g_TYPE_SPEECHBANK) {
            type = ParsingType::kSpeechbank;
        } else {
            return { RuleParserReturnCode::kInvalidValue, "Type \"" + typeStr + "\" not supported" };
        }
    } else {
        type = ParsingType::kDefault;
    }
    return g_RESULT_SUCCESS;
}

RuleParserResult parseSymbols(std::unordered_map<std::string, std::shared_ptr<Token>>& symbols, const rapidjson::Value& root, const std::optional<ParsedGroup>& parsedParent) {
    if (parsedParent) {
        symbols.insert(parsedParent->symbols.begin(), parsedParent->symbols.end());
    }

    return g_RESULT_SUCCESS;
}

RuleParserResult parseCategories(ParsedData& parsedData, std::unordered_map<std::string, RuleInfo>& namedRules, const rapidjson::Value& root, const std::unordered_map<std::string, std::shared_ptr<Token>>& symbols) {
    if (root.HasMember(g_KEY_CATEGORIES)) {
        const auto& value = root[g_KEY_CATEGORIES];
        if (!value.IsArray()) {
            return { RuleParserReturnCode::kInvalidType, "Key \"" + g_KEY_CATEGORIES + "\" must be an array" };
        }

    }
    return g_RESULT_SUCCESS;
}

RuleParserResult parseGroup(ParsedData& parsedData, const rapidjson::Value& root, const std::string& name, const std::optional<ParsedGroup>& parsedParent) {
    // Type
    ParsingType type;
    auto result = getParsingType(type, root);
    if(result.code != RuleParserReturnCode::kSuccess) {
        return result;
    }

    // Symbols
    std::unordered_map<std::string, std::shared_ptr<Token>> symbols;
    result = parseSymbols(symbols, root, parsedParent);
    if(result.code != RuleParserReturnCode::kSuccess) {
        return result;
    }

    std::unordered_map<std::string, RuleInfo> namedRules;
    result = parseCategories(parsedData, namedRules, root, symbols);
    if(result.code != RuleParserReturnCode::kSuccess) {
        return result;
    }

    parsedData.parsedGroups.insert({name, {symbols, namedRules}});
    return g_RESULT_SUCCESS;
}

// Main methods

RuleParserResult readGroup(ParsedData& parsedData, const std::string& path, bool allowMissingParent) {
    rapidjson::Document document;
    bool fileExists = readFile(path, document);
    if(!fileExists) {
        return { RuleParserReturnCode::kInvalidPath, "No file found at path \"" + path + "\"" };
    }
    if (document.HasParseError()) {
        size_t offset = document.GetErrorOffset();
        std::string errorMsg = GetParseError_En(document.GetParseError());
        return { RuleParserReturnCode::kInvalidSyntax, "Invalid JSON at offset " + std::to_string(offset) + ": " + errorMsg };
    }

    // Name
    if (!document.HasMember(g_KEY_NAME)) {
        return { RuleParserReturnCode::kMissingKey, "Group must specify key \"" + g_KEY_NAME + "\"" };
    }
    std::string name;
    auto result = getString(name, document, g_KEY_NAME);
    if(result.code != RuleParserReturnCode::kSuccess) {
        return result;
    }

    // Parent
    std::optional<ParsedGroup> parsedParent;
    if (document.HasMember(g_KEY_PARENT)) {
        std::string parentName;
        result = getString(parentName, document, g_KEY_PARENT);
        if(result.code != RuleParserReturnCode::kSuccess) {
            return result;
        }
        if(parsedData.parsedGroups.find(parentName) == parsedData.parsedGroups.end()) {
            if(!allowMissingParent) {
                // Parent is not loaded yet, add to queue
                parsedData.queuedGroups.push({document, name, parentName});
                return {RuleParserReturnCode::kParentNotYetLoaded, ""};
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

    auto dir = std::filesystem::recursive_directory_iterator(dirPath);
    for (const auto& file : dir) {
        std::string extension = file.path().extension().string();
        if(extension == g_EXT_JSON) {
            const std::string& pathStr = file.path().string();
            RuleParserResult result = readGroup(parsedData, pathStr, false);

            if(result.code == RuleParserReturnCode::kSuccess) {
                std::cout << "Successfully parsed " << pathStr << "\n";
                ++parsedData.stats.numLoaded;
            } else if(result.code != RuleParserReturnCode::kParentNotYetLoaded) {
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

RuleParserResult loadGroup(RuleDatabase& database, const std::string& path) {
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