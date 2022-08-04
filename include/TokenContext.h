#pragma once

#include "Token.h"

namespace Contextual {

class TokenContext : public Token {
public:
    TokenContext(std::string table, std::string key);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::string m_table;
    const std::string m_key;
};

}