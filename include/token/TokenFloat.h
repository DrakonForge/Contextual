#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenFloat : public SymbolToken {
public:
    TokenFloat(float value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] std::string toString() const override;
private:
    const float m_value;
};

}