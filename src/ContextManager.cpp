#include "ContextManager.h"

namespace Contextual {

ContextManager::ContextManager(std::unique_ptr<FunctionTable> functionTable) : m_functionTable(std::move(functionTable)) {}

StringTable& ContextManager::getStringTable() {
    return m_stringTable;
}
const std::unique_ptr<FunctionTable>& ContextManager::getFunctionTable() const {
    return m_functionTable;
}

}  // namespace Contextual