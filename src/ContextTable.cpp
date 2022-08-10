#include "ContextTable.h"

namespace Contextual {

namespace {
const std::unique_ptr<std::unordered_set<int>> g_NOT_FOUND = nullptr;
}

ContextTable::ContextTable(ContextManager& manager) : m_manager(manager) {}

void ContextTable::set(const std::string& key, const std::string& strValue) {
    int symbol = m_manager.getStringTable().cache(strValue);
    FactTuple tuple = {FactType::kString, static_cast<float>(symbol)};
    m_basicContext.emplace(key, tuple);
}

void ContextTable::set(const std::string& key, const char* strValue) {
    set(key, std::string(strValue));
}

void ContextTable::set(const std::string& key, float floatValue) {
    FactTuple tuple = {FactType::kNumber, floatValue};
    m_basicContext.emplace(key, tuple);
}

void ContextTable::set(const std::string& key, int intValue) {
    FactTuple tuple = {FactType::kNumber, static_cast<float>(intValue)};
    m_basicContext.emplace(key, tuple);
}

void ContextTable::set(const std::string& key, bool boolValue) {
    FactTuple tuple = {FactType::kBoolean, static_cast<float>(boolValue)};
    m_basicContext.emplace(key, tuple);
}

void ContextTable::set(const std::string& key,
                       std::unique_ptr<std::unordered_set<int>>& listValue) {
    if (!m_listContext) {
        m_listContext = std::unordered_map<std::string, std::unique_ptr<std::unordered_set<int>>>();
    }
    m_listContext->emplace(key, std::move(listValue));
}

void ContextTable::set(const std::string& key,
                       const std::unordered_set<const char*>& listValue) {
    auto intValues = std::make_unique<std::unordered_set<int>>();
    StringTable& symbolTable = m_manager.getStringTable();
    for (const char* cStr : listValue) {
        intValues->insert(symbolTable.cache(std::string(cStr)));
    }
    set(key, intValues);
}

void ContextTable::set(const std::string& key,
                       const std::unordered_set<std::string>& listValue) {
    auto intValues = std::make_unique<std::unordered_set<int>>();
    StringTable& symbolTable = m_manager.getStringTable();
    for (const std::string& str : listValue) {
        intValues->insert(symbolTable.cache(str));
    }
    set(key, intValues);
}

std::optional<std::string> ContextTable::getString(
    const std::string& key) const {
    std::optional<FactTuple> tuple = getTuple(key, FactType::kString);
    if (!tuple) {
        return std::nullopt;
    }
    int value = (int)tuple->value;
    return m_manager.getStringTable().lookup(value);
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

const std::unique_ptr<std::unordered_set<int>>& ContextTable::getList(
    const std::string& key) const {
    if (!m_listContext || m_listContext->find(key) == m_listContext->end()) {
        return g_NOT_FOUND;
    }
    return m_listContext->at(key);
}

std::optional<std::unordered_set<std::string>> ContextTable::toStringList(
    const std::string& key) const {
    const auto& intList = getList(key);
    if (intList == nullptr) {
        return std::nullopt;
    }
    std::unordered_set<std::string> strList;
    StringTable& symbolTable = m_manager.getStringTable();
    for (auto value : *intList) {
        strList.insert(symbolTable.lookup(value).value_or("NULL"));
    }
    return strList;
}

FactType ContextTable::getType(const std::string& key) const {
    if (m_listContext && m_listContext->find(key) != m_listContext->end()) {
        return FactType::kList;
    }
    auto got = m_basicContext.find(key);
    if (got == m_basicContext.end()) {
        return FactType::kNull;
    }
    const FactTuple& tuple = got->second;
    return tuple.type;
}

std::optional<float> ContextTable::getRawValue(const std::string& key) const {
    auto got = m_basicContext.find(key);
    if(got == m_basicContext.end()) {
        return std::nullopt;
    }
    const FactTuple& tuple = got->second;
    return tuple.value;
}

bool ContextTable::hasKey(const std::string& key) const {
    return (m_basicContext.find(key) != m_basicContext.end()) ||
           (m_listContext && m_listContext->find(key) != m_listContext->end());
}

std::optional<ContextTable::FactTuple> ContextTable::getTuple(
    const std::string& key, const FactType type) const {
    auto got = m_basicContext.find(key);
    if (got == m_basicContext.end()) {
        return std::nullopt;
    }
    const FactTuple& tuple = got->second;
    if (tuple.type != type) {
        return std::nullopt;
    }
    return tuple;
}

}