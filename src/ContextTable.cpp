#include "ContextTable.h"

namespace Contextual {

ContextTable::ContextTable(ContextManager& manager) : m_manager(manager) {

}

void ContextTable::set(const std::string& key, const std::string& strValue) {
    int symbol = m_manager.getSymbolTable().cache(strValue);
    FactTuple tuple = { FactType::kString, static_cast<float>(symbol) };
    m_basicContext.insert({ key, tuple });
}

void ContextTable::set(const std::string& key, const char* strValue) {
    set(key, std::string(strValue));
}

void ContextTable::set(const std::string& key, float floatValue) {
    FactTuple tuple = { FactType::kNumber, floatValue };
    m_basicContext.insert({ key, tuple });
}

void ContextTable::set(const std::string& key, int intValue) {
    FactTuple tuple = { FactType::kNumber, static_cast<float>(intValue) };
    m_basicContext.insert({ key, tuple });
}

void ContextTable::set(const std::string& key, bool boolValue) {
    FactTuple tuple = { FactType::kBoolean, static_cast<float>(boolValue) };
    m_basicContext.insert({ key, tuple });
}

void ContextTable::set(const std::string& key,
                       const std::unordered_set<int>& listValue) {
    if (!m_listContext) {
        m_listContext = std::unordered_map<std::string, std::unordered_set<int>>();
    }
    m_listContext->insert({ key, listValue });
}

void ContextTable::set(const std::string& key,
                       const std::unordered_set<const char*>& listValue) {
    std::unordered_set<int> intValues;
    SymbolTable& symbolTable = m_manager.getSymbolTable();
    for (const char* cStr : listValue) {
        intValues.insert(symbolTable.cache(std::string(cStr)));
    }
    set(key, intValues);

}

void ContextTable::set(const std::string& key,
                       const std::unordered_set<std::string>& listValue) {
    std::unordered_set<int> intValues;
    SymbolTable& symbolTable = m_manager.getSymbolTable();
    for (const std::string& str : listValue) {
        intValues.insert(symbolTable.cache(str));
    }
    set(key, intValues);
}

std::optional<std::string> ContextTable::getString(const std::string& key) const {
    std::optional<FactTuple> tuple = getTuple(key, FactType::kString);
    if (!tuple) {
        return std::nullopt;
    }
    int value = (int) tuple->value;
    return m_manager.getSymbolTable().lookup(value);
}

std::optional<float> ContextTable::getFloat(const std::string& key) const {
    std::optional<FactTuple> tuple = getTuple(key, FactType::kNumber);
    if (!tuple) {
        return std::nullopt;
    }
    return tuple->value;
}

std::optional<int> ContextTable::getInt(const std::string& key) const {
    std::optional<FactTuple> tuple = getTuple(key, FactType::kNumber);
    if (!tuple) {
        return std::nullopt;
    }
    return tuple->value;
}

std::optional<bool> ContextTable::getBool(const std::string& key) const {
    std::optional<FactTuple> tuple = getTuple(key, FactType::kBoolean);
    if (!tuple) {
        return std::nullopt;
    }
    return tuple->value != 0.0f;
}

std::optional<std::unordered_set<int>> ContextTable::getList(
    const std::string& key) const {
    if (!m_listContext || m_listContext->find(key) == m_listContext->end()) {
        return std::nullopt;
    }
    return m_listContext->at(key);
}

std::optional<std::unordered_set<std::string>> ContextTable::getStringList(
    const std::string& key) const {
    auto intList = getList(key);
    if (!intList) {
        return std::nullopt;
    }
    std::unordered_set<std::string> strList;
    SymbolTable& symbolTable = m_manager.getSymbolTable();
    for (auto value : *intList) {
        strList.insert(symbolTable.lookup(value).value_or("NULL"));
    }
    return strList;
}

std::optional<ContextTable::FactTuple> ContextTable::getTuple(const std::string& key, FactType type) const {
    if (m_basicContext.find(key) == m_basicContext.end()) {
        return std::nullopt;
    }
    const FactTuple& tuple = m_basicContext.at(key);
    if (tuple.type != type) {
        return std::nullopt;
    }
    return tuple;
}

}