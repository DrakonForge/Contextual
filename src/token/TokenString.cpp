#include "TokenString.h"

#include <utility>

namespace Contextual {

TokenString::TokenString(std::string value) : m_value(std::move(value)) {}

std::optional<std::string> TokenString::evaluate(DatabaseQuery& query) const {
    return m_value;
}

std::string TokenString::toString() const {
    return "[String=\"" + m_value + "\"]";
}

TokenType TokenString::getType() const {
    return TokenType::kString;
}

const std::string& TokenString::getValue() const {
    return m_value;
}

}  // namespace Contextual