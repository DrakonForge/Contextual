#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/MessageOnlyFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "AppSettings.h"
#include "ContextParser.h"
#include "DatabaseParser.h"
#include "DefaultFunctionTable.h"
#include "QueryParser.h"
#include "RuleDatabase.h"
#include "SpeechGenerator.h"

const std::string g_OP_QUERY = "query";
const std::string g_OP_TEST = "test";

const std::string g_QUERY_BEST = "best";
const std::string g_QUERY_ALL = "all";

const std::unordered_map<std::string, std::function<void(AppSettings&)>> NoArgHandles = {
    {"--debug", [](AppSettings& s) { s.debug = true; }},
    {"--noprint", [](AppSettings& s) { s.print = false; }},
    {"--tokenize", [](AppSettings& s) { s.tokenize = true; }},
    {"--priority", [](AppSettings& s) { s.showPriority = true; }},
    {"--simple", [](AppSettings& s) { s.simple = true; }}};

const std::unordered_map<std::string, std::function<void(AppSettings&, const std::string&)>> OneArgHandles = {
    {"--input", [](AppSettings& s, const std::string& arg) { s.inputDir = arg; }},
    {"--group", [](AppSettings& s, const std::string& arg) { s.group = arg; }},
    {"--category", [](AppSettings& s, const std::string& arg) { s.category = arg; }},
    {"--query", [](AppSettings& s, const std::string& arg) { s.queryType = arg; }},
    {"--count", [](AppSettings& s, const std::string& arg) { s.count = std::stoi(arg); }}};

AppSettings parseFlags(int argc, const char* argv[]) {
    AppSettings flags;

    for (int i = 1; i < argc; ++i) {
        std::string option = argv[i];

        auto noArg = NoArgHandles.find(option);
        if (noArg != NoArgHandles.end()) {
            noArg->second(flags);
            continue;
        }

        auto oneArg = OneArgHandles.find(option);
        if (oneArg != OneArgHandles.end()) {
            if (++i < argc) {
                oneArg->second(flags, argv[i]);
            } else {
                std::cerr << "Missing parameter after " << option << std::endl;
            }
            continue;
        }

        // Positional arguments
        if (!flags.operation) {
            flags.operation = argv[i];
        } else if (!flags.testFile) {
            flags.testFile = argv[i];
        } else {
            std::cerr << "Unrecognized command line option " << option << std::endl;
        }
    }

    return flags;
}

void printUsage(const std::string& name) {
    PLOG_INFO << "Usage: " << name << " <operation>\n\n"
              << g_OP_QUERY
              << " <query_json> [path_to_rule_database]\n"
                 "\tQueries the rule database with a group, category, and context.\n";
}

std::string tokensToString(const std::vector<std::shared_ptr<Contextual::SpeechToken>>& speechTokens) {
    std::string output;
    for (const auto& token : speechTokens) {
        output += token->toString();
    }
    return output;
}

bool loadDatabase(std::shared_ptr<Contextual::RuleDatabase>& database, const AppSettings& settings) {
    // Create function table
    std::unique_ptr<Contextual::FunctionTable> functionTable = std::make_unique<Contextual::DefaultFunctionTable>();
    functionTable->initialize();

    // Create context manager
    std::shared_ptr<Contextual::ContextManager> contextManager =
        std::make_shared<Contextual::ContextManager>(std::move(functionTable));
    database = std::make_shared<Contextual::RuleDatabase>(contextManager);
    Contextual::DatabaseParser::DatabaseStats stats =
        Contextual::DatabaseParser::loadDatabase(*database, settings.inputDir);
    return stats.numLoaded > 0 && stats.numFailed == 0;
}

void doQuery(AppSettings& settings, const std::string& name) {
    if (!settings.testFile) {
        PLOG_ERROR << "Must provide a query JSON file";
        printUsage(name);
        return;
    }

    // Load database
    std::shared_ptr<Contextual::RuleDatabase> database;
    bool success = loadDatabase(database, settings);
    if (!success) {
        PLOG_ERROR << "Failed to load database";
        return;
    }

    // Load context tables
    std::unordered_map<std::string, std::shared_ptr<Contextual::ContextTable>> contextTables;
    auto result = Contextual::App::ContextParser::parseContextTables(contextTables, database->getContextManager(),
                                                                     *settings.testFile);
    if (result.code != Contextual::JsonParseReturnCode::kSuccess) {
        PLOG_ERROR << "Error while parsing contexts: " << result.errorMsg;
        return;
    }

    // Load query info from file
    result = Contextual::App::QueryParser::parseQuery(settings, *settings.testFile);
    if (result.code != Contextual::JsonParseReturnCode::kSuccess) {
        PLOG_ERROR << "Error while parsing query: " << result.errorMsg;
        return;
    }

    if (!settings.group || !settings.category) {
        PLOG_ERROR << "Group and category must be specified";
        return;
    }

    // Create query object
    Contextual::DatabaseQuery query(database->getContextManager(), *settings.group, *settings.category);

    if (settings.queryType == g_QUERY_BEST) {
        for (int i = 0; i < settings.count; ++i) {
            Contextual::BestMatch bestMatch;
            Contextual::QueryReturnCode queryReturnCode = database->queryBestMatch(bestMatch, query);
            if (queryReturnCode != Contextual::QueryReturnCode::kSuccess) {
                PLOG_ERROR_IF(settings.print) << "No matching rule";
                continue;
            }
            std::vector<std::shared_ptr<Contextual::TextToken>> speechLine;
            std::shared_ptr<Contextual::ResponseSpeech> speechResponse;
            Contextual::SpeechGenerator::SpeechGeneratorReturnCode speechGeneratorReturnCode =
                Contextual::SpeechGenerator::performSpeechResponse(speechLine, speechResponse, query,
                                                                   bestMatch.response);
            if (speechGeneratorReturnCode == Contextual::SpeechGenerator::SpeechGeneratorReturnCode::kSelectionError) {
                PLOG_ERROR_IF(settings.print) << "No valid speech response in matching rule";
                continue;
            }
            if(settings.tokenize) {
                // Don't care about generated speech line
                PLOG_INFO_IF(settings.print) << tokensToString(speechResponse->getRandomLine());
            } else {
                PLOG_INFO_IF(settings.print) << "\"" + Contextual::SpeechGenerator::getRawSpeechLine(speechLine) + "\"";
            }
        }
    } else if (settings.queryType == g_QUERY_ALL) {
        // TODO Query all
    } else {
        PLOG_ERROR << "Unrecognized query type \"" << settings.queryType << "\"";
    }
}

void doTest(AppSettings& settings, const std::string& name) {
    PLOG_INFO << "TEST";
}

// https://blog.nickelp.ro/posts/min-guide-to-cli/
int main(int argc, const char* argv[]) {
    AppSettings settings = parseFlags(argc, argv);

    // Initializer
    static plog::ColorConsoleAppender<plog::MessageOnlyFormatter> consoleAppender;
    if (settings.debug) {
        plog::init(plog::verbose, &consoleAppender);
    } else {
        plog::init(plog::info, &consoleAppender);
    }

    // Help menu
    if (!settings.operation) {
        printUsage(argv[0]);
        return -1;
    }

    if (*settings.operation == g_OP_QUERY) {
        doQuery(settings, argv[0]);
    } else if (*settings.operation == g_OP_TEST) {
        doTest(settings, argv[0]);
    } else {
        PLOG_ERROR << "Unrecognized operation \"" << *settings.operation << "\"";
        printUsage(argv[0]);
    }
    return 0;
}