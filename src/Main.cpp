#include <filesystem>
#include <iterator>
#include <vector>

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/MessageOnlyFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include "ContextTable.h"
#include "DatabaseParser.h"
#include "DefaultFunctionTable.h"
#include "RuleDatabase.h"
#include "SpeechGenerator.h"
#include "SpeechToken.h"

std::shared_ptr<Contextual::ContextManager> createDefaultContextManager() {
    std::unique_ptr<Contextual::FunctionTable> functionTable = std::make_unique<Contextual::DefaultFunctionTable>();
    functionTable->initialize();
    std::shared_ptr<Contextual::ContextManager> contextManager =
        std::make_shared<Contextual::ContextManager>(std::move(functionTable));
    return contextManager;
}

void testDatabaseLoading() {
    std::shared_ptr<Contextual::ContextManager> contextManager = createDefaultContextManager();
    std::filesystem::path path = std::filesystem::path("..") / std::filesystem::path("data");
    Contextual::RuleDatabase database(contextManager);
    Contextual::DatabaseParser::DatabaseStats stats = Contextual::DatabaseParser::loadDatabase(database, path.string());
    PLOG_INFO << "Successful: " << stats.numLoaded;
    PLOG_INFO << "Failed: " << stats.numFailed;
    PLOG_INFO << "# Tables: " << stats.numTables;
    PLOG_INFO << "# Rules: " << stats.numRules;
    PLOG_INFO << "# Criteria Objects: " << Contextual::Criterion::getCount();
    PLOG_INFO << "StringTable Size: " << contextManager->getStringTable().getSize();

    PLOG_INFO << "";
    const std::unique_ptr<Contextual::RuleTable>& table = database.getRuleTable("Person", "Interact");
    PLOG_INFO << "Nullptr: " << (table == nullptr);
}

void testResponseQueries(int numTimes, bool tokenize) {
    // Load database
    std::shared_ptr<Contextual::ContextManager> contextManager = createDefaultContextManager();
    std::filesystem::path path = std::filesystem::path("..") / std::filesystem::path("data");
    Contextual::RuleDatabase database(contextManager);
    Contextual::DatabaseParser::loadDatabase(database, path.string());

    // Create query
    Contextual::DatabaseQuery query(contextManager, "Person", "Interact");
    // TODO: Builder pattern for context table?
    std::shared_ptr<Contextual::ContextTable> factionTable = std::make_shared<Contextual::ContextTable>(contextManager);
    factionTable->set("IsFriendly", true);
    std::shared_ptr<Contextual::ContextTable> listenerTable =
        std::make_shared<Contextual::ContextTable>(contextManager);
    listenerTable->set("Name", "jeff");
    listenerTable->set("Count", 0);
    query.addContextTable("Faction", factionTable);
    query.addContextTable("Listener", listenerTable);

    // Query database
    for (int i = 0; i < numTimes; ++i) {
        if (tokenize) {
            std::shared_ptr<Contextual::ResponseSpeech> speechResponse = database.queryBestSpeechLineResponse(query);
            if (speechResponse == nullptr) {
                PLOG_ERROR << "Failed to get tokens";
            } else {
                std::vector<std::shared_ptr<Contextual::SpeechToken>> speechTokens = speechResponse->getRandomLine();
                // Print out tokens
                std::string output;
                for (const auto& token : speechTokens) {
                    output += token->toString();
                }
                PLOG_INFO << output;

                // Print out line
                std::vector<std::shared_ptr<Contextual::TextToken>> speechLine;
                auto returnCode = Contextual::SpeechGenerator::generateLineFromTokens(speechLine, query, speechTokens);
                if (returnCode == Contextual::SpeechGeneratorReturnCode::kSuccess) {
                    PLOG_INFO << "\"" + Contextual::SpeechGenerator::getRawSpeechLine(speechLine) + "\"";
                } else {
                    PLOG_ERROR << "Failed to generate line";
                }
            }
        } else {
            std::vector<std::shared_ptr<Contextual::TextToken>> speechLine;
            Contextual::QueryReturnCode result = database.queryBestSpeechLine(speechLine, query);
            if (result != Contextual::QueryReturnCode::kSuccess) {
                PLOG_ERROR << "Failed to generate line";
            } else {
                PLOG_INFO << "\"" + Contextual::SpeechGenerator::getRawSpeechLine(speechLine) + "\"";
            }
        }
    }
}

void testLogger() {
    PLOG_VERBOSE << "verbose";
    PLOG_DEBUG << "debug";
    PLOG_INFO << "info";
    PLOG_WARNING << "warning";
    PLOG_ERROR << "error";
    PLOG_FATAL << "fatal";
    PLOG_NONE << "none";
}

int main() {
    static plog::ColorConsoleAppender<plog::MessageOnlyFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);
    PLOG_INFO << "Initializing tool...";

    //testLogger();
    //testDatabaseLoading();
    testResponseQueries(5, false);
    return 0;
}