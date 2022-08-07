#include "token/TokenString.h"

#include <utility>

namespace Contextual {

TokenString::TokenString(std::string value) : m_value(std::move(value)) {}

std::optional<std::string> TokenString::evaluate(const DatabaseQuery& query) const {
    return m_value;
}
std::string TokenString::toString() const {
    return "[String=\"" + m_value + "\"]";
}

}