#pragma once

#include <memory>
#include <vector>

#include "SymbolToken.h"

// TODO this has more to do with speech/text than symbols, ignore this class for now
namespace Contextual {

class TokenGroup : public SymbolToken {
public:
    explicit TokenGroup(std::vector<std::shared_ptr<SymbolToken>> tokens);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] std::string toString() const override;
private:
    const std::vector<std::shared_ptr<SymbolToken>> m_tokens;
};

}