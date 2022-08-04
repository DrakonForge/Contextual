#pragma once

#include "ContextTable.h"

namespace Contextual {

class DatabaseQuery {
public:
    FactType getContextType(const std::string table, const std::string key) const;
};

}