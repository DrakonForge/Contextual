#include <filesystem>
#include <iostream>
#include <iterator>

#include "ContextTable.h"
#include "DatabaseParser.h"
#include "RuleDatabase.h"
#include "StringTable.h"

void print(std::unordered_set<int> const &s)
{
    std::copy(s.begin(),
              s.end(),
              std::ostream_iterator<int>(std::cout, " "));
}

void print(std::unordered_set<std::string> const &s)
{
    std::copy(s.begin(),
              s.end(),
              std::ostream_iterator<std::string>(std::cout, " "));
}

void testContext() {
    Contextual::ContextManager contextManager;
    Contextual::StringTable& stringTable = contextManager.getStringTable();
    int a = stringTable.cache("Hello world");
    int b = stringTable.cache("Hello world");
    int c = stringTable.cache("Hello!");
    std::cout << a << " " << b << " " << c << "\n";

    std::shared_ptr<Contextual::ContextTable> contextTable = contextManager.createContextTable();
    contextTable->set("Health", 0.75f);
    contextTable->set("NumApples", 3);
    contextTable->set("Name", "Steve");
    contextTable->set("IsAlive", true);
    contextTable->set("Equipment", std::unordered_set<const char*>{ "iron", "sword", "melee" });

    std::cout << "Health as Float: " << contextTable->getFloat("Health").value_or(-999) << "\n";
    std::cout << "Health as Int: " << contextTable->getInt("Health").value_or(-999) << "\n";
    std::cout << "NumApples as Float: " << contextTable->getFloat("NumApples").value_or(-999) << "\n";
    std::cout << "NumApples as Int: " << contextTable->getInt("NumApples").value_or(-999) << "\n";
    std::cout << "Name: " << contextTable->getString("Name").value_or("-999") << "\n";
    std::cout << "IsAlive: " << contextTable->getBool("IsAlive").value_or(-999) << "\n";
    std::cout << "Equipment: ";
    print(contextTable->toStringList("Equipment").value_or(std::unordered_set<std::string>()));
    std::cout << "\n";
}

void testDatabaseLoading() {
    Contextual::ContextManager contextManager;
    std::filesystem::path path = std::filesystem::path("..") / std::filesystem::path("data");
    Contextual::RuleDatabase database(contextManager);
    Contextual::RuleParser::DatabaseStats stats = Contextual::RuleParser::loadDatabase(database, path.string());
    std::cout << "Successful: " << stats.numLoaded << "\n";
    std::cout << "Failed: " << stats.numFailed << "\n";
    std::cout << "# Tables: " << stats.numTables << "\n";
    std::cout << "# Rules: " << stats.numRules << "\n";
    std::cout << "# Criteria Objects: " << Contextual::Criterion::getCount() << "\n";
    std::cout << "StringTable Size: " << contextManager.getStringTable().getSize() << "\n";

    std::cout << "\n";
    const std::unique_ptr<Contextual::RuleTable>& table = database.getRuleTable("Person", "Interact");
    std::cout << "Nullptr: " << (table == nullptr) << "\n";
}

int main() {
    testDatabaseLoading();
    return 0;
}