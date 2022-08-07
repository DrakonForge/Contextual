#pragma once

#include <optional>
#include <string>

#include "DatabaseQuery.h"
#include "SpeechToken.h"

namespace Contextual {

class SymbolToken : public SpeechToken {
public:
    virtual std::optional<std::string> evaluate(const DatabaseQuery& query) const = 0;
protected:
    SymbolToken() = default;
};

}