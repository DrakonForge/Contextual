#pragma once

#include "Token.h"

namespace Contextual {

class TokenFloat : public Token {
public:
    TokenFloat(float value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const float m_value;
};

}