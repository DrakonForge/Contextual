#pragma once

#include <memory>

#include "ContextTable.h"
#include "SymbolTable.h"

namespace Contextual {

class ContextTable;
class ContextManager {
public:
    ContextManager() = default;
    virtual ~ContextManager() = default;
    std::shared_ptr<ContextTable> createContextTable();
    SymbolTable& getSymbolTable();
private:
    SymbolTable m_symbolTable;
};

}