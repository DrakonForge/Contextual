#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenInt : public SymbolToken {
public:
    explicit TokenInt(int value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] std::string toString() const override;
private:
    const int m_value;
};

}