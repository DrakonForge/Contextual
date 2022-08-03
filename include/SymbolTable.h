#pragma once

#include <string>
#include <optional>
#include <unordered_map>

namespace Contextual {

class SymbolTable {
public:
    SymbolTable();
    virtual ~SymbolTable() = default;

    int cache(const std::string& str);
    std::optional<std::string> lookup(int symbol) const;
    size_t getSize() const;
private:
    std::unordered_map<std::string, int32_t> m_cache;
    std::unordered_map<int32_t, std::string> m_lookup;
    int32_t m_nextId;
};

}


