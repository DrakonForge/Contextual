#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ContextManager.h"
#include "ContextTable.h"
#include "Response.h"
#include "StringTable.h"

namespace Contextual {

class DatabaseQuery {
public:
    enum class WillFail : uint8_t { kNormal, kAlways, kNever };

    explicit DatabaseQuery(ContextManager& manager);
    std::shared_ptr<ContextTable> getContextTable(const std::string& tableName) const;
    void setWillFail(WillFail value);
    WillFail willFail() const;
    StringTable& getStringTable();

private:
    ContextManager& m_manager;
    const std::unordered_map<std::string, std::shared_ptr<ContextTable>> m_contexts;
    WillFail m_willFail;
};

}  // namespace Contextual