#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ContextTable.h"
#include "Response.h"

namespace Contextual {

class DatabaseQuery {
public:
    enum class WillFail : uint8_t {
        kNormal,
        kAlways,
        kNever
    };

    DatabaseQuery();
    std::shared_ptr<ContextTable> getContextTable(const std::string& tableName) const;
    void setWillFail(WillFail value);
    WillFail willFail() const;
private:
    const std::unordered_map<std::string, std::shared_ptr<ContextTable>> m_contexts;
    WillFail m_willFail;
};

}