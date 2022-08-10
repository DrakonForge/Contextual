#include "token/TokenContext.h"

#include <utility>

#include "ContextTable.h"

namespace Contextual {

TokenContext::TokenContext(std::string table, std::string key) : m_table(std::move(table)), m_key(std::move(key)) {}

std::optional<std::string> TokenContext::evaluate(const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(m_table);
    if(contextTable != nullptr) {
        FactType type = contextTable->getType(m_key);
        if(type == FactType::kString) {
            const std::optional<std::string>& value = contextTable->getString(m_key);
            if(value) {
                return value;
            }
        }
        if(type == FactType::kNumber) {
            const std::optional<int>& value = contextTable->getInt(m_key);
            if(value) {
                // TODO: GET AS INTEGER WORD
                return std::to_string(*value);
            }
        }
    }
    return std::nullopt;
}

std::string TokenContext::toString() const {
    return "[Context=" + m_table + "." + m_key + "]";
}

TokenType TokenContext::getType() const {
    return TokenType::kAny;
}

}