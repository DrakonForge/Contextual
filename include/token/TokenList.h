#pragma once

#include <memory>
#include <vector>

#include "Token.h"

namespace Contextual {

class TokenList : public Token {
public:
    explicit TokenList(std::vector<std::shared_ptr<Token>> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::vector<std::shared_ptr<Token>> m_tokens;
};

}