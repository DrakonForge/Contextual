#include "TokenContext.h"

#include <utility>

#include "ContextTable.h"
#include "MathUtils.h"
#include "SpeechGenerator.h"

namespace Contextual {

TokenContext::TokenContext(std::string table, std::string key) : m_table(std::move(table)), m_key(std::move(key)) {}

std::optional<std::string> TokenContext::evaluate(DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(m_table);
    if (contextTable != nullptr) {
        FactType type = contextTable->getType(m_key);
        if (type == FactType::kString) {
            const std::optional<std::string>& value = contextTable->getString(m_key);
            if (value) {
                return value;
            }
        } else if (type == FactType::kNumber) {
            const std::optional<int>& value = contextTable->getInt(m_key);
            if (value) {
                return SpeechGenerator::integerToWord(*value);
            }
        } else if (type == FactType::kList) {
            std::optional<std::vector<std::string>> options = contextTable->toStringList(m_key);
            if (options) {
                size_t index = MathUtils::randUInt(0, options->size() - 1);
                return (*options)[index];
            }
        }
    }
    return std::nullopt;
}

std::string TokenContext::toString() const {
    return "[Context=" + m_table + "." + m_key + "]";
}

TokenType TokenContext::getType() const {
    return TokenType::kContext;
}

const std::string& TokenContext::getTable() const {
    return m_table;
}

const std::string& TokenContext::getKey() const {
    return m_key;
}

}  // namespace Contextual