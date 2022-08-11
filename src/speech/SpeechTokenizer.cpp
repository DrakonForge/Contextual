#include "SpeechTokenizer.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include "SpeechToken.h"
#include "TextToken.h"
#include "TokenContext.h"
#include "TokenFunction.h"
#include "TokenString.h"

namespace Contextual::SpeechTokenizer {

namespace {

const char g_SPACE = ' ';
const char g_STAR = '*';
const char g_ESCAPE = '\\';
const char g_LINEBREAK = '/';
const char g_PAUSE = '_';
const char g_SYMBOL_START = '@';
const char g_CONTEXT_START = '#';
const char g_FORMAT_START = '{';
const char g_FORMAT_SEP = ',';
const char g_FORMAT_ASSIGN = '=';
const char g_FORMAT_END = '}';
const char g_CONTEXT_SEP = '.';
const char g_ARGS_START = '(';
const char g_ARGS_END = ')';

const std::string g_ATTR_PAUSE = "pause";
const std::string g_ATTR_LINEBREAK = "linebreak";
const std::string g_ATTR_BOLD = "bold";
const std::string g_ATTR_ITALICS = "italics";

const std::shared_ptr<TextToken> g_TOKEN_LINEBREAK = std::make_shared<TextFormat>(g_ATTR_LINEBREAK);
const std::shared_ptr<TextToken> g_TOKEN_BOLD_ON = std::make_shared<TextFormatBool>(g_ATTR_BOLD, true);
const std::shared_ptr<TextToken> g_TOKEN_BOLD_OFF = std::make_shared<TextFormatBool>(g_ATTR_BOLD, false);
const std::shared_ptr<TextToken> g_TOKEN_ITALICS_ON = std::make_shared<TextFormatBool>(g_ATTR_ITALICS, true);
const std::shared_ptr<TextToken> g_TOKEN_ITALICS_OFF = std::make_shared<TextFormatBool>(g_ATTR_ITALICS, false);

const SpeechTokenizerResult g_RESULT_SUCCESS = {SpeechTokenizerReturnCode::kSuccess, ""};

void deleteTrailingSpaces(std::string& str) {
    while (!str.empty() && str[str.size() - 1] == g_SPACE) {
        str.pop_back();
    }
}

void finishStringToken(std::string& str, std::vector<std::shared_ptr<SpeechToken>>& tokens) {
    if (str.empty()) {
        return;
    }
    tokens.push_back(std::make_shared<TokenString>(std::move(str)));
    str.clear();
}

bool isIdChar(char c) {
    return std::isalpha(c) || std::isdigit(c) || c == '_';
}

SpeechTokenizerResult tokenizePause(int& index, std::vector<std::shared_ptr<SpeechToken>>& tokens,
                                    const std::string& text) {
    int pauseLength = 1;
    ++index;

    while (index < text.size()) {
        if (text[index] == g_PAUSE) {
            ++pauseLength;
            ++index;
        } else {
            tokens.push_back(std::make_shared<TextFormatInt>(g_ATTR_PAUSE, pauseLength));
            return g_RESULT_SUCCESS;
        }
    }
    return {SpeechTokenizerReturnCode::kInvalidFormat, "Line should not end on a pause"};
}

SpeechTokenizerResult tokenizeFunction(int& index, std::vector<std::shared_ptr<SpeechToken>>& tokens,
                                       const std::string& text, std::string functionName,
                                       const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                       const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                                       const FunctionTable& functionTable) {
    if (functionName.empty()) {
        return {SpeechTokenizerReturnCode::kInvalidSyntax, "Function name cannot be empty"};
    }

    // TODO: Function argument stuff
    //tokens.push_back(std::make_shared<TokenFunction>(std::move(functionName), ))
    return g_RESULT_SUCCESS;
}

SpeechTokenizerResult tokenizeSymbol(int& index, std::vector<std::shared_ptr<SpeechToken>>& tokens,
                                     const std::string& text,
                                     const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                                     const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                                     const FunctionTable& functionTable) {
    std::string symbolName;
    ++index;
    while (index < text.size()) {
        char c = text[index];
        if (isIdChar(c)) {
            symbolName.push_back(c);
            ++index;
        } else if (c == g_ARGS_START) {
            // It's a function!
            return tokenizeFunction(index, tokens, text, std::move(symbolName), symbols, localSymbols, functionTable);
        } else {
            break;
        }
    }

    if (symbolName.empty()) {
        return {SpeechTokenizerReturnCode::kInvalidSyntax, "Symbol name cannot be empty"};
    }

    // Look for the corresponding symbol
    auto got = symbols.find(symbolName);
    if (got == symbols.end()) {
        got = localSymbols.find(symbolName);
        if (got == localSymbols.end()) {
            return {SpeechTokenizerReturnCode::kInvalidSymbol,
                    "Symbol \"" + symbolName + "\" does not exist in this hierarchy"};
        }
    }

    // We found it, yay!
    tokens.push_back(got->second);
    return g_RESULT_SUCCESS;
}

SpeechTokenizerResult tokenizeContext(int& index, std::vector<std::shared_ptr<SpeechToken>>& tokens,
                                      const std::string& text) {
    std::string table;
    char c;
    ++index;
    // Parse table
    while (index < text.size()) {
        c = text[index];
        if (isIdChar(c)) {
            table.push_back(c);
            ++index;
        } else {
            if (table.empty()) {
                return {SpeechTokenizerReturnCode::kInvalidSyntax, "Context table cannot be empty"};
            }
            break;
        }
    }
    if (c != g_CONTEXT_SEP) {
        return {SpeechTokenizerReturnCode::kInvalidSyntax, "Context must specify both table and key"};
    }

    std::string key;
    ++index;
    // Parse key
    while (index < text.size()) {
        c = text[index];
        if (isIdChar(c)) {
            key.push_back(c);
            ++index;
        } else {
            break;
        }
    }

    if (key.empty()) {
        return {SpeechTokenizerReturnCode::kInvalidSyntax, "Context key cannot be empty"};
    }
    tokens.push_back(std::make_shared<TokenContext>(std::move(table), std::move(key)));
    return g_RESULT_SUCCESS;
}

SpeechTokenizerResult validateAttribute(const std::string& attribute) {
    if (attribute == g_ATTR_BOLD) {
        return {SpeechTokenizerReturnCode::kInvalidFormat, "Bold formatting should use '**' instead of being explicit"};
    }
    if (attribute == g_ATTR_ITALICS) {
        return {SpeechTokenizerReturnCode::kInvalidFormat,
                "Italics formatting should use '*' instead of being explicit"};
    }
    if (attribute == g_ATTR_PAUSE) {
        return {SpeechTokenizerReturnCode::kInvalidFormat, "Pause formatting should use '_' instead of being explicit"};
    }
    if (attribute == g_ATTR_LINEBREAK) {
        return {SpeechTokenizerReturnCode::kInvalidFormat,
                "Line break formatting should use '/' instead of being explicit"};
    }
    return g_RESULT_SUCCESS;
}

SpeechTokenizerResult tokenizeAttribute(std::vector<std::shared_ptr<SpeechToken>>& tokens, std::string& attribute,
                                        std::string& value) {
    if (value == "true") {
        tokens.push_back(std::make_shared<TextFormatBool>(std::move(attribute), true));
        return g_RESULT_SUCCESS;
    }
    if (value == "false") {
        tokens.push_back(std::make_shared<TextFormatBool>(std::move(attribute), false));
        return g_RESULT_SUCCESS;
    }

    int intValue;
    bool success = false;
    try {
        intValue = std::stoi(value);
        success = true;
    } catch (std::invalid_argument& e) {
        // Not an integer
    } catch (std::out_of_range& e) {
        // Not an integer
    }
    if (success) {
        tokens.push_back(std::make_shared<TextFormatInt>(std::move(attribute), intValue));
        return g_RESULT_SUCCESS;
    }

    float floatValue;
    // success is guaranteed to be false
    try {
        floatValue = std::stof(value);
        success = true;
        return g_RESULT_SUCCESS;
    } catch (std::invalid_argument& e) {
        // Not a float
    } catch (std::out_of_range& e) {
        // Not a float
    }
    if (success) {
        tokens.push_back(std::make_shared<TextFormatFloat>(std::move(attribute), floatValue));
        return g_RESULT_SUCCESS;
    }

    // Doesn't match anything else--guess it's a string
    tokens.push_back(std::make_shared<TextFormatString>(std::move(attribute), std::move(value)));
    return g_RESULT_SUCCESS;
}

SpeechTokenizerResult tokenizeFormat(int& index, std::vector<std::shared_ptr<SpeechToken>>& tokens,
                                     const std::string& text) {
    ++index;
    std::string str;
    std::string attribute;
    bool consumeSpaces = true;

    while (index < text.size()) {
        char c = text[index];
        if (c != g_SPACE) {
            consumeSpaces = false;
        }

        if (c == g_FORMAT_ASSIGN) {
            deleteTrailingSpaces(str);
            if (!attribute.empty()) {
                return {SpeechTokenizerReturnCode::kInvalidSyntax, "Cannot set multiple attributes at once"};
            }
            if (str.empty()) {
                return {SpeechTokenizerReturnCode::kInvalidSyntax, "Attribute name cannot be empty"};
            }
            attribute = str;
            auto result = validateAttribute(attribute);
            if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                return result;
            }
            str.clear();
            consumeSpaces = true;
            ++index;
        } else if (c == g_FORMAT_SEP || c == g_FORMAT_END) {
            // Add attribute token and start parsing a new one
            deleteTrailingSpaces(str);
            // Expect comma or end of list
            if (str.empty()) {
                if (attribute.empty()) {
                    return {SpeechTokenizerReturnCode::kInvalidSyntax, "Attribute cannot be empty"};
                }
                return {SpeechTokenizerReturnCode::kInvalidSyntax, "Attribute value cannot be empty"};
            }
            if (attribute.empty()) {
                // Attribute without value
                validateAttribute(str);
                tokens.push_back(std::make_shared<TextFormat>(std::move(str)));
                str.clear();
            } else {
                // Decide if bool, int, float, or string
                auto result = tokenizeAttribute(tokens, attribute, str);
                if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                    return result;
                }
            }

            ++index;
            if (c == g_FORMAT_END) {
                // Exit
                return g_RESULT_SUCCESS;
            } else {
                // Separator, keep going
                attribute.clear();
                str.clear();
                consumeSpaces = true;
            }
        } else if (c == g_SPACE && consumeSpaces) {
            ++index;
        } else if (c == g_ESCAPE) {
            // Escape
            if (index >= text.size() - 1) {
                return {SpeechTokenizerReturnCode::kInvalidSyntax,
                        "Escape character must be followed by another character"};
            }
            char next = text[index + 1];
            str.push_back(next);
            index += 2;
        } else {
            // Normal character
            str.push_back(c);
            ++index;
        }
    }

    return {SpeechTokenizerReturnCode::kInvalidSyntax, "Formatting block not closed properly"};
}

SpeechTokenizerResult validate(const std::vector<std::shared_ptr<SpeechToken>>& tokens, const bool italics,
                               const bool bold) {
    if (tokens.empty()) {
        return {SpeechTokenizerReturnCode::kInvalidFormat, "Line should not be empty"};
    }
    if (italics) {
        return {SpeechTokenizerReturnCode::kInvalidFormat, "Italics formatting not closed properly"};
    }
    if (bold) {
        return {SpeechTokenizerReturnCode::kInvalidFormat, "Italics formatting not closed properly"};
    }
    return g_RESULT_SUCCESS;
}

}  // namespace

SpeechTokenizerResult tokenize(std::vector<std::shared_ptr<SpeechToken>>& tokens, const std::string& text,
                               const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                               const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                               const FunctionTable& functionTable) {
    std::string nextString;
    int index = 0;
    int numStars = 0;
    bool italics = false;
    bool bold = false;
    bool consumeSpaces = false;

    while (index < text.size()) {
        char c = text[index];

        // Continue consuming spaces if the character is a space
        consumeSpaces = consumeSpaces && c == g_SPACE;

        // Toggle italics if only one star
        if (c != g_STAR && numStars > 0) {
            if (italics) {
                tokens.push_back(g_TOKEN_ITALICS_OFF);
            } else {
                tokens.push_back(g_TOKEN_ITALICS_ON);
            }
            italics = !italics;
            numStars = 0;
        }

        if (consumeSpaces) {
            ++index;
        } else if (c == g_ESCAPE) {
            if (index >= text.size() - 1) {
                return {SpeechTokenizerReturnCode::kInvalidSyntax,
                        "Escape character must be followed by another character"};
            }
            char next = text[index + 1];
            nextString.push_back(next);
            index += 2;
        } else if (c == g_LINEBREAK) {
            if (index >= text.size() - 1) {
                return {SpeechTokenizerReturnCode::kInvalidFormat, "Line should not end on a line break"};
            }
            deleteTrailingSpaces(nextString);
            if (nextString.empty()) {
                return {SpeechTokenizerReturnCode::kInvalidFormat, "Line break should not separate blank lines"};
            }
            finishStringToken(nextString, tokens);
            tokens.push_back(g_TOKEN_LINEBREAK);
            consumeSpaces = true;
            ++index;
        } else if (c == g_PAUSE) {
            if (!nextString.empty() && nextString[nextString.size() - 1] == g_SPACE) {
                return {SpeechTokenizerReturnCode::kInvalidFormat, "Spaces should be to the right of a pause"};
            }
            finishStringToken(nextString, tokens);
            auto result = tokenizePause(index, tokens, text);
            if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                return result;
            }
        } else if (c == g_SYMBOL_START) {
            finishStringToken(nextString, tokens);
            auto result = tokenizeSymbol(index, tokens, text, symbols, localSymbols, functionTable);
            if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                return result;
            }
        } else if (c == g_CONTEXT_START) {
            finishStringToken(nextString, tokens);
            auto result = tokenizeContext(index, tokens, text);
            if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                return result;
            }
        } else if (c == g_FORMAT_START) {
            finishStringToken(nextString, tokens);
            auto result = tokenizeFormat(index, tokens, text);
            if (result.code != SpeechTokenizerReturnCode::kSuccess) {
                return result;
            }
        } else if (c == g_STAR) {
            finishStringToken(nextString, tokens);
            ++numStars;
            ++index;
            // Toggle bold if two stars
            if (numStars == 2) {
                if (bold) {
                    tokens.push_back(g_TOKEN_BOLD_OFF);
                } else {
                    tokens.push_back(g_TOKEN_BOLD_ON);
                }
                bold = !bold;
                numStars = 0;
            }
        } else {
            // Normal character
            nextString.push_back(c);
            ++index;
        }
    }

    // Last character
    if (italics && numStars == 1) {
        tokens.push_back(g_TOKEN_ITALICS_OFF);
        italics = false;
    }
    finishStringToken(nextString, tokens);
    return validate(tokens, italics, bold);
}

}  // namespace Contextual::SpeechTokenizer