#include "token/TokenFloat.h"

#include "SpeechGenerator.h"

namespace Contextual {

TokenFloat::TokenFloat(float value) : m_value(value) {}

std::optional<std::string> TokenFloat::evaluate(DatabaseQuery& query) const {
    int intValue = static_cast<int>(m_value);
    return SpeechGenerator::integerToWord(intValue);
}

std::string TokenFloat::toString() const {
    return "[Float=" + std::to_string(m_value) + "]";
}

TokenType TokenFloat::getType() const {
    return TokenType::kFloat;
}
float TokenFloat::getValue() const {
    return m_value;
}

}  // namespace Contextual