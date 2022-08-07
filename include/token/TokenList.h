#pragma once

#include <memory>
#include <vector>

#include "SymbolToken.h"

namespace Contextual {

class TokenList : public SymbolToken {
public:
    explicit TokenList(std::vector<std::shared_ptr<SymbolToken>> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::vector<std::shared_ptr<SymbolToken>> m_tokens;
};

}