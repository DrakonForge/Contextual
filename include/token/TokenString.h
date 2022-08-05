#pragma once

#include "Token.h"

namespace Contextual {

class TokenString : public Token {
public:
    TokenString(std::string value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::string m_value;
};

}