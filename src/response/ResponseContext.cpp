#include "ResponseContext.h"

namespace Contextual {

ResponseContext::ResponseContext(std::string table, std::string key)
    : m_table(std::move(table)), m_key(std::move(key)) {}

ResponseType ResponseContext::getType() const {
    return ResponseType::kContext;
}

}  // namespace Contextual