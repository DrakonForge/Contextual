#include "DatabaseQuery.h"

namespace Contextual {

FactType Contextual::DatabaseQuery::getContextType(
    const std::string table, const std::string key) const {
    return FactType::kNull;
}

}
