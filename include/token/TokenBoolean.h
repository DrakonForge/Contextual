#pragma once

#include "Token.h"

namespace Contextual {

class TokenBoolean : public Token {
public:
    explicit TokenBoolean(bool value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const bool m_value;
};

}