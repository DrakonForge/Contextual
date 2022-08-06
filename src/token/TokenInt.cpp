#include "token/TokenInt.h"

namespace Contextual {

TokenInt::TokenInt(int value) : m_value(value) {}

std::optional<std::string> TokenInt::evaluate(const DatabaseQuery& query) const {
    // TODO: GET AS INTEGER WORD
    return std::to_string(m_value);
}

}