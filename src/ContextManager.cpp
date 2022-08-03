#include "ContextManager.h"

namespace Contextual {

std::shared_ptr<ContextTable> ContextManager::createContextTable() {
    auto contextTable = std::make_shared<ContextTable>(*this);
    return contextTable;
}

SymbolTable& ContextManager::getSymbolTable() { return m_symbolTable; }

}