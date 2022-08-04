#include "TokenContext.h"

#include <utility>

#include "ContextTable.h"

namespace Contextual {

TokenContext::TokenContext(std::string table, std::string key) : m_table(std::move(table)), m_key(std::move(key)) {}

std::optional<std::string> TokenContext::evaluate(const DatabaseQuery& query) const {
    FactType type = query.getContextType(m_table, m_key);
    if(type == FactType::kString) {
        // TODO: GET AS STRING
        return std::nullopt;
    }
    if(type == FactType::kNumber) {
        // TODO: GET AS INTEGER WORD
        return std::nullopt;
    }
    // Other tokens should not be evaluated to string
    return std::nullopt;
}

}