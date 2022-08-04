#pragma once

#include <vector>

#include "Token.h"

namespace Contextual {

class TokenList : public Token {
public:
    explicit TokenList(std::vector<Token> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::vector<Token> m_tokens;
};

}