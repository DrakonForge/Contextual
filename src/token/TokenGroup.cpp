#include "token/TokenGroup.h"

#include <utility>

namespace Contextual {

TokenGroup::TokenGroup(std::vector<std::shared_ptr<SymbolToken>> tokens) : m_tokens(std::move(tokens)) {}

std::optional<std::string> TokenGroup::evaluate(const DatabaseQuery& query) const {
    std::string result;
    std::optional<std::string> nextTokenStr;
    for(const auto& token : m_tokens) {
        nextTokenStr = token->evaluate(query);
        if(nextTokenStr) {
            result += *nextTokenStr;
        } else {
            return std::nullopt;
        }
    }
    return result;
}

}