#include "ContextManager.h"

namespace Contextual {

ContextManager::ContextManager(const FunctionTable& functionTable) : m_functionTable(functionTable) {}

std::shared_ptr<ContextTable> ContextManager::createContextTable() {
    auto contextTable = std::make_shared<ContextTable>(*this);
    return contextTable;
}

StringTable& ContextManager::getStringTable() {
    return m_stringTable;
}
const FunctionTable& ContextManager::getFunctionTable() const {
    return m_functionTable;
}

}  // namespace Contextual