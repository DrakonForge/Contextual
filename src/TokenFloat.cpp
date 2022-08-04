#include "TokenFloat.h"

namespace Contextual {

TokenFloat::TokenFloat(float value) : m_value(value) {}

std::optional<std::string> TokenFloat::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}

}