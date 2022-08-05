#include "RuleTable.h"

#include <algorithm>

namespace Contextual {

namespace {
const std::unique_ptr<RuleEntry> g_NOT_FOUND = nullptr;

// Descending order
bool compareEntries(const std::unique_ptr<RuleEntry>& entry1, const std::unique_ptr<RuleEntry>& entry2) {
    return entry1->priority > entry2->priority;
}

}

void RuleTable::addEntry(std::unique_ptr<RuleEntry>& ruleEntry) {
    m_entries.push_back(std::move(ruleEntry));
    m_sorted = false;
}

bool RuleTable::sortEntries() {
    if(m_sorted) {
        return false;
    }
    std::sort(m_entries.begin(), m_entries.end(), compareEntries);
    return true;
}

size_t RuleTable::getNumEntries() {
    return m_entries.size();
}

const std::unique_ptr<RuleEntry>& RuleTable::query(DatabaseQuery query) {
    if (m_entries.empty()) {
        return g_NOT_FOUND;
    }
    return m_entries[0];
}

}