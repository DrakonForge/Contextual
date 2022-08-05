#include "token/TokenBoolean.h"

namespace Contextual {

TokenBoolean::TokenBoolean(bool value) : m_value(value) {}

std::optional<std::string> TokenBoolean::evaluate(const DatabaseQuery& query) const {
    // Boolean tokens should not be evaluated to string
    return std::nullopt;
}

}