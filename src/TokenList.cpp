#include "TokenList.h"

namespace Contextual {

TokenList::TokenList(std::vector<std::shared_ptr<Token>> tokens) : m_tokens(std::move(tokens)) {}

std::optional<std::string> TokenList::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}

}