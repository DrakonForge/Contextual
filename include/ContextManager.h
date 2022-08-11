#pragma once

#include <memory>

#include "ContextTable.h"
#include "StringTable.h"

namespace Contextual {

class ContextTable;
class ContextManager {
public:
    ContextManager() = default;
    virtual ~ContextManager() = default;
    std::shared_ptr<ContextTable> createContextTable();
    StringTable& getStringTable();

private:
    StringTable m_stringTable;
};

}  // namespace Contextual