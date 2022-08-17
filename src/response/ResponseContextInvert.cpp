#include "ResponseContextInvert.h"

namespace Contextual {

ResponseContextInvert::ResponseContextInvert(std::string table, std::string key)
    : ResponseContext(std::move(table), std::move(key)) {}

void ResponseContextInvert::execute(DatabaseQuery& query) {
    const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(m_table);
    if (contextTable != nullptr) {
        std::optional<bool> boolValue = contextTable->getBool(m_key);
        if (boolValue) {
            contextTable->set(m_key, !(*boolValue));
        }
    }
}

}  // namespace Contextual