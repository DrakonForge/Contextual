#include <gtest/gtest.h>

#include <memory>
#include <unordered_map>

#include "ContextManager.h"
#include "ContextTable.h"
#include "DefaultFunctionTable.h"

namespace Contextual {

class ContextTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_contextTable =
            std::make_shared<ContextTable>(std::make_shared<ContextManager>(std::make_unique<DefaultFunctionTable>()));
    }

    std::shared_ptr<ContextTable> m_contextTable;
};

TEST_F(ContextTableTest, TestNullValue) {
    EXPECT_FALSE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kNull);
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
}

TEST_F(ContextTableTest, TestCStringValue) {
    m_contextTable->set("Key", "Value");

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kString);
    EXPECT_NE(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getString("Key"), "Value");
}

TEST_F(ContextTableTest, TestStringValue) {
    std::string value = "Value";
    m_contextTable->set("Key", value);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kString);
    // Raw value is based on StringTable and can be arbitrary (but shouldn't be nothing)
    EXPECT_NE(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getString("Key"), "Value");
}

TEST_F(ContextTableTest, TestFloatValue) {
    m_contextTable->set("Key", 1.5f);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kNumber);
    EXPECT_EQ(m_contextTable->getRawValue("Key"), 1.5f);
    EXPECT_EQ(m_contextTable->getFloat("Key"), 1.5f);
    EXPECT_EQ(m_contextTable->getInt("Key"), 1);
}

TEST_F(ContextTableTest, TestIntValue) {
    m_contextTable->set("Key", 2);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kNumber);
    EXPECT_EQ(m_contextTable->getRawValue("Key"), 2.0f);
    EXPECT_EQ(m_contextTable->getFloat("Key"), 2.0f);
    EXPECT_EQ(m_contextTable->getInt("Key"), 2);
}

TEST_F(ContextTableTest, TestBoolValue) {
    m_contextTable->set("Key", true);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kBoolean);
    EXPECT_EQ(m_contextTable->getRawValue("Key"), 1.0f);
    EXPECT_EQ(m_contextTable->getBool("Key"), true);
}

TEST_F(ContextTableTest, TestIntList) {
    std::unique_ptr<std::unordered_set<int>> list = std::make_unique<std::unordered_set<int>>();
    list->insert(1);
    list->insert(2);
    list->insert(3);
    m_contextTable->set("Key", list);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kList);
    EXPECT_FALSE(m_contextTable->isStringList("Key"));
    // Lists are stored separately, so they do not have a raw value
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getList("Key")->size(), 3);
}

TEST_F(ContextTableTest, TestIntListExplicit) {
    std::unique_ptr<std::unordered_set<int>> list = std::make_unique<std::unordered_set<int>>();
    list->insert(1);
    list->insert(2);
    list->insert(3);
    m_contextTable->set("Key", list, false);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kList);
    EXPECT_FALSE(m_contextTable->isStringList("Key"));
    // Lists are stored separately, so they do not have a raw value
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getList("Key")->size(), 3);
}

TEST_F(ContextTableTest, TestStringList) {
    std::unordered_set<std::string> list;
    list.insert("A");
    list.insert("B");
    list.insert("C");
    m_contextTable->set("Key", list);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kList);
    EXPECT_TRUE(m_contextTable->isStringList("Key"));
    // Lists are stored separately, so they do not have a raw value
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getList("Key")->size(), 3);

    std::optional<std::vector<std::string>> stringList = m_contextTable->toStringList("Key");
    EXPECT_TRUE(stringList);
    EXPECT_EQ(stringList->size(), 3);
    // Order does not matter
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "A") != stringList->end());
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "B") != stringList->end());
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "C") != stringList->end());
}

TEST_F(ContextTableTest, TestCStringList) {
    std::unordered_set<const char*> list;
    list.insert("A");
    list.insert("B");
    list.insert("C");
    m_contextTable->set("Key", list);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kList);
    EXPECT_TRUE(m_contextTable->isStringList("Key"));
    // Lists are stored separately, so they do not have a raw value
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getList("Key")->size(), 3);

    std::optional<std::vector<std::string>> stringList = m_contextTable->toStringList("Key");
    EXPECT_TRUE(stringList);
    EXPECT_EQ(stringList->size(), 3);
    // Order does not matter
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "A") != stringList->end());
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "B") != stringList->end());
    EXPECT_TRUE(std::find(stringList->begin(), stringList->end(), "C") != stringList->end());
}

TEST_F(ContextTableTest, TestOverwriteBasic) {
    std::string value = "Value";
    m_contextTable->set("Key", value);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kString);
    EXPECT_EQ(m_contextTable->getString("Key"), "Value");
    EXPECT_EQ(m_contextTable->getFloat("Key"), std::nullopt);

    m_contextTable->set("Key", 1.5f);
    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kNumber);
    EXPECT_EQ(m_contextTable->getString("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getFloat("Key"), 1.5f);
}

TEST_F(ContextTableTest, TestOverwriteList) {
    std::string value = "Value";
    m_contextTable->set("Key", value);

    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kString);
    EXPECT_EQ(m_contextTable->getString("Key"), "Value");
    EXPECT_EQ(m_contextTable->getList("Key"), nullptr);

    std::unique_ptr<std::unordered_set<int>> list = std::make_unique<std::unordered_set<int>>();
    list->insert(1);
    list->insert(2);
    list->insert(3);
    m_contextTable->set("Key", list);
    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kList);
    EXPECT_EQ(m_contextTable->getString("Key"), std::nullopt);
    EXPECT_NE(m_contextTable->getList("Key"), nullptr);
    EXPECT_EQ(m_contextTable->getList("Key")->size(), 3);
}

}  // namespace Contextual