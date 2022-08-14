#include <filesystem>
#include <iostream>
#include <iterator>
#include <vector>

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
    Contextual::RuleParser::DatabaseStats stats = Contextual::RuleParser::loadDatabase(database, path.string());
    std::cout << "Successful: " << stats.numLoaded << "\n";
    std::cout << "Failed: " << stats.numFailed << "\n";
    std::cout << "# Tables: " << stats.numTables << "\n";
    std::cout << "# Rules: " << stats.numRules << "\n";
    std::cout << "# Criteria Objects: " << Contextual::Criterion::getCount() << "\n";
    std::cout << "StringTable Size: " << contextManager->getStringTable().getSize() << "\n";

    std::cout << "\n";
    const std::unique_ptr<Contextual::RuleTable>& table = database.getRuleTable("Person", "Interact");
    std::cout << "Nullptr: " << (table == nullptr) << "\n";
}

void testResponseQueries(int numTimes, bool tokenize) {
    // Load database
    std::shared_ptr<Contextual::ContextManager> contextManager = createDefaultContextManager();
    std::filesystem::path path = std::filesystem::path("..") / std::filesystem::path("data");
    Contextual::RuleDatabase database(contextManager);
    Contextual::RuleParser::DatabaseStats stats = Contextual::RuleParser::loadDatabase(database, path.string());

    // Create query
    Contextual::DatabaseQuery query(contextManager, "Person", "Interact");
    // TODO: Builder pattern for context table?
    std::shared_ptr<Contextual::ContextTable> factionTable = std::make_shared<Contextual::ContextTable>(contextManager);
    factionTable->set("IsFriendly", true);
    std::shared_ptr<Contextual::ContextTable> listenerTable =
        std::make_shared<Contextual::ContextTable>(contextManager);
    listenerTable->set("Name", "jeff");
    query.addContextTable("Faction", factionTable);
    query.addContextTable("Listener", listenerTable);

    // Query database
    for (int i = 0; i < numTimes; ++i) {
        std::string output;
        if (tokenize) {
            std::shared_ptr<Contextual::ResponseSpeech> speechResponse = database.queryBestSpeechLineResponse(query);
            if (speechResponse == nullptr) {
                output += "Failed to get tokens\n";
            } else {
                std::vector<std::shared_ptr<Contextual::SpeechToken>> speechTokens = speechResponse->getRandomLine();
                // Print out tokens
                for (const auto& token : speechTokens) {
                    output += token->toString();
                }
                output += "\n";

                // Print out line
                std::vector<std::shared_ptr<Contextual::TextToken>> speechLine;
                auto returnCode = Contextual::SpeechGenerator::generateLine(speechLine, query, speechTokens);
                if (returnCode == Contextual::SpeechGeneratorReturnCode::kSuccess) {
                    output += "\"" + Contextual::SpeechGenerator::getRawSpeechLine(speechLine) + "\"\n";
                } else {
                    output += "Failed to generate line\n";
                }
            }
        } else {
            std::vector<std::shared_ptr<Contextual::TextToken>> speechLine;
            Contextual::QueryReturnCode result = database.queryBestSpeechLine(speechLine, query);
            if (result != Contextual::QueryReturnCode::kSuccess) {
                output += "Failed to generate line\n";
            } else {
                output += "\"" + Contextual::SpeechGenerator::getRawSpeechLine(speechLine) + "\"\n";
            }
        }
        std::cout << output;
    }
}

int main() {
    // testDatabaseLoading();
    testResponseQueries(5, true);
    return 0;
}