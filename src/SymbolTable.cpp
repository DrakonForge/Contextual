#include "SymbolTable.h"

namespace Contextual {

namespace {
const uint32_t g_STARTING_ID = 1000;
}

SymbolTable::SymbolTable() {
    m_nextId = g_STARTING_ID;
}

int SymbolTable::cache(const std::string& str) {
    if (m_cache.find(str) == m_cache.end()) {
        m_cache.insert({ str, m_nextId});
        m_lookup.insert({m_nextId, str});
        return m_nextId++;
    }
    return m_cache.at(str);
}

std::optional<std::string> SymbolTable::lookup(int symbol) const {
    if (m_lookup.find(symbol) == m_lookup.end()) {
        return std::nullopt;
    }
    return m_lookup.at(symbol);
}

size_t SymbolTable::getSize() const {
    return m_cache.size();
}

}
