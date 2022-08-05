#pragma once

#include <memory>
#include <vector>

#include "Token.h"

namespace Contextual {

class TokenFunction : public Token {
public:
    TokenFunction(std::string name, std::vector<std::shared_ptr<Token>> args);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::string m_name;
    const std::vector<std::shared_ptr<Token>> m_args;
};

}