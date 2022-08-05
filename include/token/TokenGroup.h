#pragma once

#include <memory>
#include <vector>

#include "Token.h"

// TODO this has more to do with speech/text than symbols, ignore this class for now
namespace Contextual {

class TokenGroup : public Token {
public:
    explicit TokenGroup(std::vector<std::shared_ptr<Token>> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
private:
    const std::vector<std::shared_ptr<Token>> m_tokens;
};

}