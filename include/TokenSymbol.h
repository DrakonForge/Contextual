#pragma once

#include "Token.h"

namespace Contextual {

class TokenSymbol : public Token {
public:
    TokenSymbol(std::string  name);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::string m_name;
};

}