#include "token/TokenBoolean.h"

namespace Contextual {

TokenBoolean::TokenBoolean(bool value) : m_value(value) {}

std::optional<std::string> TokenBoolean::evaluate(const DatabaseQuery& query) const {
    // Boolean tokens should not be evaluated to string
    return std::nullopt;
}

std::string TokenBoolean::toString() const {
    if(m_value) {
        return "[Boolean=true]";
    } else {
        return "[Boolean=false]";
    }
}

TokenType TokenBoolean::getType() const {
    return TokenType::kBool;
}

}