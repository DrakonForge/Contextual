#pragma once

#include "Token.h"

namespace Contextual {

class TokenInt : public Token {
public:
    TokenInt(int value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const int m_value;
};

}