#pragma once

#include "SymbolToken.h"

namespace Contextual {

class TokenBoolean : public SymbolToken {
public:
    explicit TokenBoolean(bool value);
    [[nodiscard]] std::optional<std::string> evaluate(const DatabaseQuery& query) const override;
    [[nodiscard]] std::string toString() const override;
private:
    const bool m_value;
};

}