#include "ContextManager.h"

namespace Contextual {

std::shared_ptr<ContextTable> ContextManager::createContextTable() {
    auto contextTable = std::make_shared<ContextTable>(*this);
    return contextTable;
}

StringTable& ContextManager::getStringTable() {
    return m_stringTable;
}

}  // namespace Contextual