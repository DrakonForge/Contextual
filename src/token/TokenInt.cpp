#include "TokenInt.h"

#include "SpeechGenerator.h"

namespace Contextual {

TokenInt::TokenInt(int value) : m_value(value) {}

std::optional<std::string> TokenInt::evaluate(DatabaseQuery& query) const {
    return SpeechGenerator::integerToWord(m_value);
}

std::string TokenInt::toString() const {
    return "[Integer=" + std::to_string(m_value) + "]";
}

TokenType TokenInt::getType() const {
    return TokenType::kInt;
}

int TokenInt::getValue() const {
    return m_value;
}

}  // namespace Contextual