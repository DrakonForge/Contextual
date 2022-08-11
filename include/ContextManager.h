#pragma once

#include <memory>

#include "ContextTable.h"
#include "FunctionTable.h"
#include "StringTable.h"

namespace Contextual {

class ContextTable;
class ContextManager {
public:
    ContextManager(const FunctionTable& functionTable);
    virtual ~ContextManager() = default;
    std::shared_ptr<ContextTable> createContextTable();
    const FunctionTable& getFunctionTable() const;
    StringTable& getStringTable();

private:
    const FunctionTable& m_functionTable;
    StringTable m_stringTable;
};

}  // namespace Contextual