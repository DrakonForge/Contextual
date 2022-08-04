#pragma once

#include <optional>
#include <string>

#include "DatabaseQuery.h"

namespace Contextual {

class Token {
public:
    virtual std::optional<std::string> evaluate(const DatabaseQuery& query) const = 0;
};

}