#pragma once

#include <memory>
#include <vector>

#include "SymbolToken.h"

namespace Contextual {

class TokenFunction : public SymbolToken {
public:
    TokenFunction(std::string name, std::vector<std::shared_ptr<SymbolToken>> args);
    [[nodiscard]] std::optional<std::string> evaluate(DatabaseQuery& query) const override;
    [[nodiscard]] TokenType getType() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] const std::vector<std::shared_ptr<SymbolToken>>& getArgs() const;

private:
    const std::string m_name;
    const std::vector<std::shared_ptr<SymbolToken>> m_args;
};

}  // namespace Contextual