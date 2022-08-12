#pragma once

#include <memory>

#include "ContextTable.h"
#include "FunctionTable.h"
#include "StringTable.h"

namespace Contextual {

class ContextTable;
class FunctionTable;

class ContextManager {
public:
    ContextManager(std::unique_ptr<FunctionTable> functionTable);
    virtual ~ContextManager() = default;
    const std::unique_ptr<FunctionTable>& getFunctionTable() const;
    StringTable& getStringTable();

private:
    const std::unique_ptr<FunctionTable> m_functionTable;
    StringTable m_stringTable;
};

}  // namespace Contextual