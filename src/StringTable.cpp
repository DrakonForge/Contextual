#include "StringTable.h"

namespace Contextual {

namespace {
const uint32_t g_STARTING_ID = 1000;
}

StringTable::StringTable() {
    m_nextId = g_STARTING_ID;
}

int StringTable::cache(const std::string& str) {
    auto got = m_cache.find(str);
    if (got == m_cache.end()) {
        m_cache.insert({ str, m_nextId});
        m_lookup.insert({m_nextId, str});
        return m_nextId++;
    }
    return got->second;
}

std::optional<std::string> StringTable::lookup(int symbol) const {
    auto got = m_lookup.find(symbol);
    if (got == m_lookup.end()) {
        return std::nullopt;
    }
    return got->second;
}

size_t StringTable::getSize() const {
    return m_cache.size();
}

}
