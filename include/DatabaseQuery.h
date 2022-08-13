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

    explicit DatabaseQuery(std::shared_ptr<ContextManager>& manager, std::string group, std::string category);
    void addContextTable(const std::string& tableName, std::shared_ptr<ContextTable> contextTable);
    std::shared_ptr<ContextTable> getContextTable(const std::string& tableName) const;
    void setWillFail(WillFail value);
    WillFail willFail() const;
    StringTable& getStringTable();
    const std::unique_ptr<FunctionTable>& getFunctionTable() const;
    const std::string& getGroup() const;
    const std::string& getCategory() const;

private:
    std::shared_ptr<ContextManager>& m_manager;
    std::string m_group;
    std::string m_category;
    std::unordered_map<std::string, std::shared_ptr<ContextTable>> m_contexts;
    WillFail m_willFail;
};

}  // namespace Contextual