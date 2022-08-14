#include <gtest/gtest.h>

#include <string>

#include "SpeechGenerator.h"

namespace Contextual {

class SpeechGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {}

    static void testIntegerToWord(int num, const std::string& integerStr, const std::string& ordinalStr) {
        std::string integerResult = SpeechGenerator::integerToWord(num);
        EXPECT_EQ(integerResult, integerStr);
        std::string ordinalResult = SpeechGenerator::integerToOrdinal(num);
        EXPECT_EQ(ordinalResult, ordinalStr);
    }
};

TEST_F(SpeechGeneratorTest, TestIntegerToWord) {
    testIntegerToWord(-1, "", "");
    testIntegerToWord(0, "zero", "zeroth");
    testIntegerToWord(1, "one", "first");
    testIntegerToWord(2, "two", "second");
    testIntegerToWord(3, "three", "third");
    testIntegerToWord(4, "four", "fourth");
    testIntegerToWord(5, "five", "fifth");
    testIntegerToWord(6, "six", "sixth");
    testIntegerToWord(7, "seven", "seventh");
    testIntegerToWord(8, "eight", "eighth");
    testIntegerToWord(9, "nine", "ninth");
    testIntegerToWord(10, "ten", "tenth");
    testIntegerToWord(11, "eleven", "eleventh");
    testIntegerToWord(12, "twelve", "twelfth");
    testIntegerToWord(100, "one hundred", "one hundredth");
    testIntegerToWord(111, "one hundred eleven", "one hundred eleventh");
    testIntegerToWord(1000, "one thousand", "one thousandth");
    testIntegerToWord(947'268'851,
                      "nine hundred forty-seven million two hundred sixty-eight thousand eight hundred fifty-one",
                      "nine hundred forty-seven million two hundred sixty-eight thousand eight hundred fifty-first");
}

}  // namespace Contextual