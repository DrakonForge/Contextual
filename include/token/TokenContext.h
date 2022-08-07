#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenContext : public SymbolToken {
public:
    TokenContext(std::string table, std::string key);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] std::string toString() const override;
private:
    const std::string m_table;
    const std::string m_key;
};

}