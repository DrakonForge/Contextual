#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenFloat : public SymbolToken {
public:
    explicit TokenFloat(float value);
    [[nodiscard]] std::optional<std::string> evaluate(DatabaseQuery& query) const override;
    [[nodiscard]] TokenType getType() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] float getValue() const;

private:
    const float m_value;
};

}  // namespace Contextual