#include "ResponseContextSetStatic.h"

namespace Contextual {

ResponseContextSetStatic::ResponseContextSetStatic(std::string table, std::string key, FactType type, float value)
    : ResponseContext(std::move(table), std::move(key)), m_type(type), m_value(value) {}

void ResponseContextSetStatic::execute(DatabaseQuery& query) {
    const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(m_table);
    if (contextTable != nullptr) {
        contextTable->setRawValue(m_key, m_value, m_type);
    }
}

}  // namespace Contextual