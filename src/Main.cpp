#include <iostream>
#include <iterator>

#include "ContextTable.h"
#include "SymbolTable.h"

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

int main() {

    Contextual::ContextManager contextManager;
    Contextual::SymbolTable& symbolTable = contextManager.getSymbolTable();
    int a = symbolTable.cache("Hello world");
    int b = symbolTable.cache("Hello world");
    int c = symbolTable.cache("Hello!");
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
    print(contextTable->getStringList("Equipment").value_or(std::unordered_set<std::string>()));
    std::cout << "\n";

    return 0;
}