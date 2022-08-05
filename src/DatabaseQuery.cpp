#include "DatabaseQuery.h"

namespace Contextual {

DatabaseQuery::DatabaseQuery() {
    m_willFail = WillFail::kNormal;
}

std::shared_ptr<ContextTable> DatabaseQuery::getContextTable(
    const std::string& tableName) const {
    if(m_contexts.find(tableName) == m_contexts.end()) {
        return nullptr;
    }
    return m_contexts.at(tableName);
}

void DatabaseQuery::setWillFail(DatabaseQuery::WillFail value) {
    m_willFail = value;
}

DatabaseQuery::WillFail DatabaseQuery::willFail() const {
    return m_willFail;
}

}
