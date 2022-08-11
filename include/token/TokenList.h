#pragma once

#include <memory>
#include <vector>

#include "SymbolToken.h"

namespace Contextual {

class TokenList : public SymbolToken {
public:
    explicit TokenList(std::vector<std::shared_ptr<SymbolToken>> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(DatabaseQuery& query) const override;
    [[nodiscard]] TokenType getType() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] const std::vector<std::shared_ptr<SymbolToken>>& getValue() const;

private:
    const std::vector<std::shared_ptr<SymbolToken>> m_tokens;
};

}  // namespace Contextual