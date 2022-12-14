#include "TokenList.h"

#include "MathUtils.h"

namespace Contextual {

TokenList::TokenList(std::vector<std::shared_ptr<SymbolToken>> tokens) : m_tokens(std::move(tokens)) {}

std::optional<std::string> TokenList::evaluate(DatabaseQuery& query) const {
    size_t i;
    return evaluateList(query, i);
}

std::optional<std::string> TokenList::evaluateList(DatabaseQuery& query, size_t& index) const {
    index = MathUtils::randUInt(0, m_tokens.size() - 1);
    const std::shared_ptr<SymbolToken>& token = m_tokens[index];
    return token->evaluate(query);
}

std::string TokenList::toString() const {
    std::string str = "[List=[" + m_tokens[0]->toString();
    for (int i = 1; i < m_tokens.size(); ++i) {
        str += ", " + m_tokens[i]->toString();
    }
    str += "]]";
    return str;
}

TokenType TokenList::getType() const {
    return TokenType::kList;
}

const std::vector<std::shared_ptr<SymbolToken>>& TokenList::getValue() const {
    return m_tokens;
}

}  // namespace Contextual