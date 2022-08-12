#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ContextManager.h"
#include "ContextTable.h"
#include "Response.h"
#include "StringTable.h"

namespace Contextual {

class ContextManager;
class ContextTable;
class FunctionTable;

class DatabaseQuery {
public:
    enum class WillFail : uint8_t { kNormal, kAlways, kNever };

    explicit DatabaseQuery(std::shared_ptr<ContextManager>& manager);
    std::shared_ptr<ContextTable> getContextTable(const std::string& tableName) const;
    void setWillFail(WillFail value);
    WillFail willFail() const;
    StringTable& getStringTable();
    const std::unique_ptr<FunctionTable>& getFunctionTable() const;

private:
    std::shared_ptr<ContextManager>& m_manager;
    const std::unordered_map<std::string, std::shared_ptr<ContextTable>> m_contexts;
    WillFail m_willFail;
};

}  // namespace Contextual