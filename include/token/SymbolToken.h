#pragma once

#include <optional>
#include <string>

#include "DatabaseQuery.h"
#include "SpeechToken.h"
#include "TokenType.h"

namespace Contextual {

class DatabaseQuery;

class SymbolToken : public SpeechToken {
public:
    [[nodiscard]] virtual std::optional<std::string> evaluate(DatabaseQuery& query) const = 0;
    [[nodiscard]] virtual TokenType getType() const = 0;
    [[nodiscard]] bool isSymbolToken() const override { return true; }

protected:
    SymbolToken() = default;
};

}  // namespace Contextual