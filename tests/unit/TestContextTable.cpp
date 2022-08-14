#include <gtest/gtest.h>

#include <memory>

#include "ContextManager.h"
#include "ContextTable.h"
#include "DefaultFunctionTable.h"

namespace Contextual {

class ContextTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_contextManager = std::make_shared<ContextManager>(std::make_unique<DefaultFunctionTable>());
        m_contextTable = std::make_shared<ContextTable>(m_contextManager);
    }
    std::shared_ptr<ContextManager> m_contextManager;
    std::shared_ptr<ContextTable> m_contextTable;
};

TEST_F(ContextTableTest, TestNull) {
    EXPECT_FALSE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kNull);
    EXPECT_EQ(m_contextTable->getRawValue("Key"), std::nullopt);
}

TEST_F(ContextTableTest, TestStringValue) {
    m_contextTable->set("Key", "Value");
    EXPECT_TRUE(m_contextTable->hasKey("Key"));
    EXPECT_EQ(m_contextTable->getType("Key"), FactType::kString);
    EXPECT_NE(m_contextTable->getRawValue("Key"), std::nullopt);
    EXPECT_EQ(m_contextTable->getString("Key"), "Value");
}

}  // namespace Contextual