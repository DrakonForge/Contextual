#pragma once

#include "ContextTable.h"

namespace Contextual {

class DatabaseQuery {
public:
    // TODO: Ability to "always fail" or "always succeed" fail criterion
    FactType getContextType(const std::string& table, const std::string& key) const;
};

}