#include "SpeechGenerator.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ResponseContext.h"
#include "ResponseMultiple.h"
#include "SymbolToken.h"
#include "TokenList.h"

namespace Contextual::SpeechGenerator {

namespace {

const std::string g_ZERO = "zero";
const std::string g_HUNDRED = "hundred ";
const std::string g_THOUSAND = "thousand ";
const std::string g_MILLION = "million ";
const std::string g_BILLION = "billion ";
const std::string g_BELOW_TWENTY[] = {"",         "one",     "two",     "three",     "four",     "five",    "six",
                                      "seven",    "eight",   "nine",    "ten",       "eleven",   "twelve",  "thirteen",
                                      "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
const std::string g_TENS[] = {"", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
const std::unordered_map<std::string, std::string> g_SPECIAL_ORDINAL_MAP = {
    {"one", "first"},    {"two", "second"}, {"three", "third"},   {"five", "fifth"},
    {"eight", "eighth"}, {"nine", "ninth"}, {"twelve", "twelfth"}};
const int g_VAL_TEN = 10;
const int g_VAL_TWENTY = 20;
const int g_VAL_HUNDRED = 100;
const int g_VAL_THOUSAND = 1000;
const int g_VAL_MILLION = 1000000;
const int g_VAL_BILLION = 1000000000;
const int g_MAX_LIST_ATTEMPTS = 5;
const int g_MAX_SPEECH_ATTEMPTS = 3;

// https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
// trim from both ends (in place)
void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
std::vector<std::string> split(const std::string& input, char delim) {
    std::vector<std::string> result;
    std::istringstream f(input);
    std::string s;
    while (getline(f, s, delim)) {
        result.push_back(s);
    }
    return result;
}

std::string integerWordToOrdinal(const std::string& numWord) {
    if (numWord.empty()) {
        return "";
    }
    std::vector<std::string> words = split(numWord, ' ');
    std::string last = words[words.size() - 1];
    std::string replace;

    if (last.find('-') != std::string::npos) {
        // Should only split into two, causes issues if there are multiple hyphens
        std::vector<std::string> lastWords = split(last, '-');
        std::string lastWithDash = lastWords[1];
        std::string lastReplace;
        auto got = g_SPECIAL_ORDINAL_MAP.find(lastWithDash);
        if (got != g_SPECIAL_ORDINAL_MAP.end()) {
            lastReplace = got->second;
        } else if (lastWithDash[lastWithDash.size() - 1] == 'y') {
            lastReplace = lastWithDash.substr(0, lastWithDash.size() - 1) + "ieth";
        } else {
            lastReplace = lastWithDash + "th";
        }
        replace = lastWords[0] + '-' + lastReplace;
    } else {
        auto got = g_SPECIAL_ORDINAL_MAP.find(last);
        if (got != g_SPECIAL_ORDINAL_MAP.end()) {
            replace = got->second;
        } else if (last[last.size() - 1] == 'y') {
            replace = last.substr(0, last.size() - 1) + "ieth";
        } else {
            replace = last + "th";
        }
    }
    words[words.size() - 1] = replace;

    // Join together results
    std::string result = words[0];
    for (int i = 1; i < words.size(); ++i) {
        result += ' ' + words[i];
    }
    return result;
}

std::string helper(const int num) {
    if (num <= 0) {
        return "";
    }
    if (num < g_VAL_TWENTY) {
        return g_BELOW_TWENTY[num] + ' ';
    }
    if (num < g_VAL_HUNDRED) {
        std::string result = g_TENS[num / g_VAL_TEN];
        int remainder = num % g_VAL_TEN;
        if (remainder > 0) {
            result += '-' + helper(remainder);
        }
        return result;
    }
    if (num < g_VAL_THOUSAND) {
        return helper(num / g_VAL_HUNDRED) + g_HUNDRED + helper(num % g_VAL_HUNDRED);
    }
    if (num < g_VAL_MILLION) {
        return helper(num / g_VAL_THOUSAND) + g_THOUSAND + helper(num % g_VAL_THOUSAND);
    }
    if (num < g_VAL_BILLION) {
        return helper(num / g_VAL_MILLION) + g_MILLION + helper(num % g_VAL_MILLION);
    }
    return helper(num / g_VAL_BILLION) + g_BILLION + helper(num % g_VAL_BILLION);
}

}  // namespace

// SelectionError -> no speech response, GenerationError -> speech response but failed to generate speech line
SpeechGeneratorReturnCode performSpeechResponse(std::vector<std::shared_ptr<TextToken>>& speechLine,
                                                std::shared_ptr<ResponseSpeech>& speechResponse, DatabaseQuery& query,
                                                const std::shared_ptr<Response>& response) {
    // Look for speech responses; there should only be one maximum
    // There should never be a nested multiple response, so it is either top level or in a multiple response
    if (response->getType() == ResponseType::kSpeech) {
        speechResponse = std::static_pointer_cast<ResponseSpeech>(response);
        if(generateLineFromResponse(speechLine, query, speechResponse)) {
            return SpeechGeneratorReturnCode::kSuccess;
        }
        return SpeechGeneratorReturnCode::kGenerationError;
    } else if (response->getType() == ResponseType::kMultiple) {
        // Go through all responses
        SpeechGeneratorReturnCode returnCode = SpeechGeneratorReturnCode::kSelectionError;
        const auto& multipleResponse = std::static_pointer_cast<ResponseMultiple>(response);
        for (const auto& responseItem : multipleResponse->getResponses()) {
            if (responseItem->getType() == ResponseType::kContext) {
                const auto& contextResponse = std::static_pointer_cast<ResponseContext>(response);
                contextResponse->execute(query);
            }
            if (responseItem->getType() == ResponseType::kSpeech) {
                // Only execute the first speech response
                if (speechResponse == nullptr) {
                    speechResponse = std::static_pointer_cast<ResponseSpeech>(responseItem);
                    if(generateLineFromResponse(speechLine, query, speechResponse)) {
                        returnCode = SpeechGeneratorReturnCode::kSuccess;
                    } else {
                        returnCode = SpeechGeneratorReturnCode::kGenerationError;
                    }
                }
            }
        }
        return returnCode;
    }
    return SpeechGeneratorReturnCode::kSelectionError;
}

bool generateLineFromResponse(std::vector<std::shared_ptr<TextToken>>& speechLine, DatabaseQuery& query,
                              const std::shared_ptr<ResponseSpeech>& speechResponse) {
    int attempts = 0;
    while (++attempts < g_MAX_SPEECH_ATTEMPTS) {
        bool result = generateLineFromTokens(speechLine, query, speechResponse->getRandomLine());
        if (result) {
            return true;
        }
    }
    return false;
}

bool generateLineFromTokens(std::vector<std::shared_ptr<TextToken>>& speechLine, DatabaseQuery& query,
                            const std::vector<std::shared_ptr<SpeechToken>>& speechTokens) {
    bool hasText = false;
    std::unordered_map<std::shared_ptr<SpeechToken>, std::unordered_set<std::string>> chosenListOptions;
    query.clearPrevChoices();
    for (const auto& token : speechTokens) {
        if (token->isSymbolToken()) {
            hasText = true;
            const auto& symbolToken = std::static_pointer_cast<SymbolToken>(token);
            std::optional<std::string> nextTokenStr;
            size_t index;  // Used only for list tokens

            // Attempt to sample without replacement for list tokens
            // If this fails, then continue with the choice (does not fail)
            if (symbolToken->getType() == TokenType::kList) {
                const auto& listToken = std::static_pointer_cast<TokenList>(symbolToken);
                int attempts = 0;
                auto got = chosenListOptions.find(symbolToken);
                if (got == chosenListOptions.end()) {
                    got = chosenListOptions.emplace(symbolToken, std::unordered_set<std::string>()).first;
                }
                while (++attempts <= g_MAX_LIST_ATTEMPTS) {
                    nextTokenStr = listToken->evaluateList(query, index);
                    if (nextTokenStr && got->second.find(*nextTokenStr) == got->second.end()) {
                        got->second.insert(*nextTokenStr);
                        break;
                    }
                }
            } else {
                nextTokenStr = symbolToken->evaluate(query);
            }

            if (nextTokenStr) {
                if (symbolToken->getType() == TokenType::kList) {
                    query.addPrevChoice(index, *nextTokenStr);
                }
                speechLine.push_back(std::make_shared<TextLiteral>(std::move(*nextTokenStr)));
            } else {
                return false;
            }
        } else {
            const auto& textToken = std::static_pointer_cast<TextToken>(token);
            speechLine.push_back(textToken);
        }
    }
    // Must have text to print properly
    if (!hasText) {
        return false;
    }
    return true;
}

std::string getRawSpeechLine(const std::vector<std::shared_ptr<TextToken>>& speechLine) {
    std::string result;
    for (const auto& textToken : speechLine) {
        if (textToken->isLiteral()) {
            const auto& textLiteral = std::static_pointer_cast<TextLiteral>(textToken);
            result += textLiteral->value;
        }
    }
    return result;
}

std::string integerToWord(const int num) {
    if (num < 0) {
        return "";
    }
    if (num == 0) {
        return g_ZERO;
    }
    std::string result = helper(num);
    trim(result);
    return result;
}

std::string integerToOrdinal(const int num) {
    std::string numWord = integerToWord(num);
    return integerWordToOrdinal(numWord);
}

}  // namespace Contextual::SpeechGenerator