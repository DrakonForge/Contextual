#include "token/TokenFloat.h"

namespace Contextual {

TokenFloat::TokenFloat(float value) : m_value(value) {}

std::optional<std::string> TokenFloat::evaluate(const DatabaseQuery& query) const {
    // TODO: GET AS INTEGER WORD
    int intValue = static_cast<int>(m_value);
    return std::to_string(intValue);
}
std::string TokenFloat::toString() const {
    return "[Float=" + std::to_string(m_value) + "]";
}

}