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
#include "SpeechTokenizer.h"
#include "StringTable.h"
#include "SymbolToken.h"

void print(std::unordered_set<int> const& s) {
    std::copy(s.begin(), s.end(), std::ostream_iterator<int>(std::cout, " "));
}

void print(std::vector<std::string> const& s) {
    std::copy(s.begin(), s.end(), std::ostream_iterator<std::string>(std::cout, " "));
}

std::shared_ptr<Contextual::ContextManager> createDefaultContextManager() {
    std::unique_ptr<Contextual::FunctionTable> functionTable = std::make_unique<Contextual::DefaultFunctionTable>();
    functionTable->initialize();
    std::shared_ptr<Contextual::ContextManager> contextManager =
        std::make_shared<Contextual::ContextManager>(std::move(functionTable));
    return contextManager;
}

void testContext() {
    std::shared_ptr<Contextual::ContextManager> contextManager = createDefaultContextManager();
    Contextual::StringTable& stringTable = contextManager->getStringTable();
    int a = stringTable.cache("Hello world");
    int b = stringTable.cache("Hello world");
    int c = stringTable.cache("Hello!");
    std::cout << a << " " << b << " " << c << "\n";

    std::shared_ptr<Contextual::ContextTable> contextTable = std::make_shared<Contextual::ContextTable>(contextManager);
    contextTable->set("Health", 0.75f);
    contextTable->set("NumApples", 3);
    contextTable->set("Name", "Steve");
    contextTable->set("IsAlive", true);
    contextTable->set("Equipment", std::unordered_set<const char*>{"iron", "sword", "melee"});

    std::cout << "Health as Float: " << contextTable->getFloat("Health").value_or(-999) << "\n";
    std::cout << "Health as Int: " << contextTable->getInt("Health").value_or(-999) << "\n";
    std::cout << "NumApples as Float: " << contextTable->getFloat("NumApples").value_or(-999) << "\n";
    std::cout << "NumApples as Int: " << contextTable->getInt("NumApples").value_or(-999) << "\n";
    std::cout << "Name: " << contextTable->getString("Name").value_or("-999") << "\n";
    std::cout << "IsAlive: " << contextTable->getBool("IsAlive").value_or(-999) << "\n";
    std::cout << "Equipment: ";
    print(contextTable->toStringList("Equipment").value_or(std::vector<std::string>()));
    std::cout << "\n";
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

void testTextParsing(const std::string& str) {
    std::shared_ptr<Contextual::ContextManager> contextManager = createDefaultContextManager();
    std::vector<std::shared_ptr<Contextual::SpeechToken>> tokens;
    const std::unordered_map<std::string, std::shared_ptr<Contextual::SymbolToken>> symbols;
    const std::unordered_map<std::string, std::shared_ptr<Contextual::SymbolToken>> localSymbols;

    std::cout << "Target string: \"" << str << "\""
              << "\n";
    auto result =
        Contextual::SpeechTokenizer::tokenize(tokens, str, symbols, localSymbols, contextManager->getFunctionTable());
    if (result.code == Contextual::SpeechTokenizerReturnCode::kSuccess) {
        std::cout << "Success!"
                  << "\n";
    } else {
        std::cout << "Error: " << result.errorMsg << "\n";
    }
    std::cout << "Tokenization: ";
    for (const auto& token : tokens) {
        std::cout << token->toString();
    }
    std::cout << "\n\n";
}

void testTextParsingMany() {
    testTextParsing("Hello world!");
    testTextParsing("*Hello world!*");
    testTextParsing("{speed=5}Hello, {speed=1}#Speaker.Name!");
    testTextParsing("Hello there...____ old friend.");
    testTextParsing("Hello there!/General Kenobi!");
    testTextParsing("Hello world!__ ");

    // Should fail
    testTextParsing("*Hello world!");
    testTextParsing("{Hello world!");
    testTextParsing("Hello world!/");
    testTextParsing("Hello world!_");
    testTextParsing("Hello world! _");
    testTextParsing("{italics=true}Hello world!{italics=false}");
}

void testIntegerToWord() {
    std::vector<int> toTest = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 100, 1000, 12055601, 124, 947268851};
    for (auto i : toTest) {
        std::cout << "Target: " << i << "\n";
        std::cout << "Word: " << Contextual::SpeechGenerator::integerToWord(i) << "\n";
        std::cout << "Ordinal: " << Contextual::SpeechGenerator::integerToOrdinal(i) << "\n\n";
    }
}

int main() {
    // testTextParsingMany();
    // testIntegerToWord();
    // testDatabaseLoading();
    testResponseQueries(5, true);
    return 0;
}