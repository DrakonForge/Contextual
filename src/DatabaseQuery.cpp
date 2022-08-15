#include "DatabaseQuery.h"

namespace Contextual {

DatabaseQuery::DatabaseQuery(std::shared_ptr<ContextManager>& contextManager, std::string group, std::string category)
    : m_manager(contextManager), m_group(std::move(group)), m_category(std::move(category)) {
    m_willFail = WillFail::kNormal;
}

void DatabaseQuery::addContextTable(const std::string& tableName, std::shared_ptr<ContextTable> contextTable) {
    m_contexts.emplace(tableName, std::move(contextTable));
}

std::shared_ptr<ContextTable> DatabaseQuery::getContextTable(const std::string& tableName) const {
    auto got = m_contexts.find(tableName);
    if (got == m_contexts.end()) {
        return nullptr;
    }
    return got->second;
}

void DatabaseQuery::setWillFail(DatabaseQuery::WillFail value) {
    m_willFail = value;
}

DatabaseQuery::WillFail DatabaseQuery::willFail() const {
    return m_willFail;
}

StringTable& DatabaseQuery::getStringTable() {
    return m_manager->getStringTable();
}

const std::unique_ptr<FunctionTable>& DatabaseQuery::getFunctionTable() const {
    return m_manager->getFunctionTable();
}

const std::string& DatabaseQuery::getGroup() const {
    return m_group;
}

const std::string& DatabaseQuery::getCategory() const {
    return m_category;
}

void DatabaseQuery::clearPrevChoices() {
    m_prevChoices.clear();
    m_prevChoiceIndices.clear();
}

void DatabaseQuery::addPrevChoice(size_t index, std::string choice) {
    m_prevChoices.push_back(std::move(choice));
    m_prevChoiceIndices.push_back(index);
}

std::optional<std::string> DatabaseQuery::getPrevChoice(int index) const {
    if(0 <= index && index < m_prevChoices.size()) {
        return m_prevChoices[index];
    }
    return std::nullopt;
}

std::optional<size_t> DatabaseQuery::getPrevChoiceIndex(int index) const {
    if(0 <= index && index < m_prevChoiceIndices.size()) {
        return m_prevChoiceIndices[index];
    }
    return std::nullopt;
}

}  // namespace Contextual
