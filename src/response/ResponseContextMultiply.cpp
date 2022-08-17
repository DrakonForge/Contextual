#include "ResponseContextMultiply.h"

namespace Contextual {

ResponseContextMultiply::ResponseContextMultiply(std::string table, std::string key, float value)
    : ResponseContext(std::move(table), std::move(key)), m_value(value) {}

void ResponseContextMultiply::execute(DatabaseQuery& query) {
    const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(m_table);
    if (contextTable != nullptr) {
        std::optional<float> numValue = contextTable->getFloat(m_key);
        if (numValue) {
            contextTable->set(m_key, *numValue * m_value);
        }
    }
}

}  // namespace Contextual