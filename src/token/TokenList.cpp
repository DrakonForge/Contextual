#include "token/TokenList.h"

#include <random>

namespace Contextual {

TokenList::TokenList(std::vector<std::shared_ptr<SymbolToken>> tokens) : m_tokens(std::move(tokens)) {}

std::optional<std::string> TokenList::evaluate(const DatabaseQuery& query) const {
    // TODO Improve RNG generation
    static std::default_random_engine e;
    std::uniform_int_distribution<size_t> dis(0, m_tokens.size());
    size_t index = dis(e);
    const std::shared_ptr<SymbolToken>& token = m_tokens[index];
    return token->evaluate(query);
}

}