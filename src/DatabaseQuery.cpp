#include "DatabaseQuery.h"

namespace Contextual {

DatabaseQuery::DatabaseQuery(std::shared_ptr<ContextManager>& contextManager) : m_manager(contextManager) {
    m_willFail = WillFail::kNormal;
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

}  // namespace Contextual
