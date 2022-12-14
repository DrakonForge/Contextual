#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenBoolean : public SymbolToken {
public:
    explicit TokenBoolean(bool value);
    [[nodiscard]] std::optional<std::string> evaluate(DatabaseQuery& query) const override;
    [[nodiscard]] TokenType getType() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] bool getValue() const;

private:
    const bool m_value;
};

}  // namespace Contextual