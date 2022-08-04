#include "TokenGroup.h"

#include <utility>

namespace Contextual {

TokenGroup::TokenGroup(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::optional<std::string> TokenGroup::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}

}