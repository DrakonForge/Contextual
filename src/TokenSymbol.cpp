#include "TokenSymbol.h"

#include <utility>

namespace Contextual {

TokenSymbol::TokenSymbol(std::string  name) : m_name(std::move(name)) {

}

std::optional<std::string> TokenSymbol::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}

}