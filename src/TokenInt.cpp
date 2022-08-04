#include "TokenInt.h"

namespace Contextual {

TokenInt::TokenInt(int value) : m_value(value) {}

std::optional<std::string> TokenInt::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}

}