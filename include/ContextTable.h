#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ContextManager.h"

namespace Contextual {

enum class FactType : uint8_t { kNull, kString, kNumber, kBoolean, kList };

class ContextManager;

class ContextTable {
public:
    explicit ContextTable(std::shared_ptr<ContextManager> manager);
    virtual ~ContextTable() = default;

    void set(const std::string& key, const std::string& strValue);
    void set(const std::string& key, const char* strValue);
    void set(const std::string& key, float floatValue);
    void set(const std::string& key, int intValue);
    void set(const std::string& key, bool boolValue);
    void set(const std::string& key, std::unique_ptr<std::unordered_set<int>> listValue);
    void set(const std::string& key, std::unique_ptr<std::unordered_set<int>> listValue, bool isStringList);
    void set(const std::string& key, const std::unordered_set<const char*>& listValue);
    void set(const std::string& key, const std::unordered_set<std::string>& listValue);
    void setRawValue(const std::string& key, float value, FactType type);
    void remove(const std::string& key);
    std::optional<std::string> getString(const std::string& key) const;
    std::optional<float> getFloat(const std::string& key) const;
    std::optional<int> getInt(const std::string& key) const;
    std::optional<bool> getBool(const std::string& key) const;
    const std::unique_ptr<std::unordered_set<int>>& getList(const std::string& key) const;
    std::optional<float> getRawValue(const std::string& key) const;
    std::optional<std::vector<std::string>> toStringList(const std::string& key) const;
    bool isStringList(const std::string& key) const;
    bool hasKey(const std::string& key) const;
    FactType getType(const std::string& key) const;

private:
    struct FactTuple {
        FactType type;
        float value;
    };

    std::shared_ptr<ContextManager> m_manager;
    std::unordered_map<std::string, FactTuple> m_basicContext;
    std::optional<std::unordered_map<std::string, std::pair<std::unique_ptr<std::unordered_set<int>>, bool>>>
        m_listContext;
    std::optional<FactTuple> getTuple(const std::string& key, FactType type) const;
};

}  // namespace Contextual