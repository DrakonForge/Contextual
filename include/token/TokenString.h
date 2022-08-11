#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenString : public SymbolToken {
public:
    explicit TokenString(std::string value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] TokenType getType() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] const std::string& getValue() const;

private:
    const std::string m_value;
};

}  // namespace Contextual