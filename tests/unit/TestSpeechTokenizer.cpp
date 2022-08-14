#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "DefaultFunctionTable.h"
#include "FunctionTable.h"
#include "SpeechTokenizer.h"
#include "SymbolToken.h"

namespace Contextual {

class SpeechTokenizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_functionTable = std::make_unique<DefaultFunctionTable>();
    }

    void testValidSpeechLine(const std::string& speechLine, const std::string& expected) {
        std::vector<std::shared_ptr<SpeechToken>> tokens;
        auto result = SpeechTokenizer::tokenize(tokens, speechLine, m_symbols, m_localSymbols, m_functionTable);
        EXPECT_EQ(result.code, SpeechTokenizerReturnCode::kSuccess);

        // Convert tokens to string
        std::string str;
        for (const auto& token : tokens) {
            str += token->toString();
        }
        EXPECT_EQ(str, expected);
    }

    void testInvalidSpeechLine(const std::string& speechLine, const SpeechTokenizerReturnCode expectedCode) {
        std::vector<std::shared_ptr<SpeechToken>> tokens;
        auto result = SpeechTokenizer::tokenize(tokens, speechLine, m_symbols, m_localSymbols, m_functionTable);
        EXPECT_EQ(result.code, expectedCode);
    }

    std::unique_ptr<FunctionTable> m_functionTable;
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>> m_symbols;
    std::unordered_map<std::string, std::shared_ptr<SymbolToken>> m_localSymbols;
};

// TODO: Add more comprehensive tests + reasons for each

TEST_F(SpeechTokenizerTest, TestValidSpeechLines) {
    testValidSpeechLine("Hello world!", R"([String="Hello world!"])");
    testValidSpeechLine("*Hello world!*", R"({italics=true}[String="Hello world!"]{italics=false})");
    testValidSpeechLine("{speed=5}Hello, {speed=1}#Speaker.Name!", R"({speed=5}[String="Hello, "]{speed=1}[Context=Speaker.Name][String="!"])");
    testValidSpeechLine("Hello there...____ old friend.", R"([String="Hello there..."]{pause=4}[String=" old friend."])");
    testValidSpeechLine("Hello there!/General Kenobi!", R"([String="Hello there!"]{linebreak}[String="General Kenobi!"])");
    testValidSpeechLine("Hello world!__ ", R"([String="Hello world!"]{pause=2}[String=" "])");
}

TEST_F(SpeechTokenizerTest, TestInvalidSyntax) {
    testInvalidSpeechLine("Hello world!\\", SpeechTokenizerReturnCode::kInvalidSyntax);
    testInvalidSpeechLine("Hello world!#", SpeechTokenizerReturnCode::kInvalidSyntax);
    testInvalidSpeechLine("Hello #Name", SpeechTokenizerReturnCode::kInvalidSyntax);
    testInvalidSpeechLine("Hello #Name.!", SpeechTokenizerReturnCode::kInvalidSyntax);
    testInvalidSpeechLine("{Hello world!", SpeechTokenizerReturnCode::kInvalidSyntax);
    testInvalidSpeechLine("Hello world!{emote=\\", SpeechTokenizerReturnCode::kInvalidSyntax);
}

TEST_F(SpeechTokenizerTest, TestInvalidFormat) {
    testInvalidSpeechLine("*Hello world!", SpeechTokenizerReturnCode::kInvalidFormat);
    testInvalidSpeechLine("Hello world!/", SpeechTokenizerReturnCode::kInvalidFormat);
    testInvalidSpeechLine("Hello world!_", SpeechTokenizerReturnCode::kInvalidFormat);
    testInvalidSpeechLine("Hello world! _", SpeechTokenizerReturnCode::kInvalidFormat);
    testInvalidSpeechLine("{italics=true}Hello world!{italics=false}", SpeechTokenizerReturnCode::kInvalidFormat);
}

}  // namespace Contextual