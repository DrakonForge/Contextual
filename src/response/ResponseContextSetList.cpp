#include "ResponseContextSetList.h"

namespace Contextual {

ResponseContextSetList::ResponseContextSetList(std::string table, std::string key, std::unordered_set<int> value,
                                               bool isStringList)
    : ResponseContext(std::move(table), std::move(key)), m_value(std::move(value)), m_isStringList(isStringList) {}

void ResponseContextSetList::execute(DatabaseQuery& query) {
    const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(m_table);
    if (contextTable != nullptr) {
        contextTable->set(m_key, std::make_unique<std::unordered_set<int>>(m_value), m_isStringList);
    }
}

}  // namespace Contextual