#pragma once

#include <vector>

#include "Token.h"

namespace Contextual {

class TokenFunction : public Token {
public:
    TokenFunction(std::string name, std::vector<Token> args);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::string m_name;
    const std::vector<Token> m_args;
};

}