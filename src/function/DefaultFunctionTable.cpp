#include "DefaultFunctionTable.h"

#include <algorithm>
#include <random>

#include "SpeechGenerator.h"

namespace Contextual {

namespace {
const std::string g_GENDER_MALE = "male";
const std::string g_GENDER_FEMALE = "female";
const std::string g_GENDER_NONE = "none";

const std::string g_SUBJECTIVE[] = {"they", "he", "she"};
const std::string g_OBJECTIVE[] = {"them", "him", "her"};
const std::string g_POSSESSIVE[] = {"theirs", "his", "hers"};
const std::string g_REFLEXIVE[] = {"themself", "himself", "herself"};

int genderToInt(const std::string& gender) {
    if (gender == g_GENDER_NONE) {
        return 0;
    }
    if (gender == g_GENDER_MALE) {
        return 1;
    }
    if (gender == g_GENDER_FEMALE) {
        return 2;
    }
    return -1;
}

/* Default Functions */
FunctionVal capitalize(std::string str) {
    if (str.empty()) {
        return FunctionVal();
    }
    str[0] = static_cast<char>(std::toupper(str[0]));
    return FunctionVal(str);
}

FunctionVal uncapitalize(std::string str) {
    if (str.empty()) {
        return FunctionVal();
    }
    str[0] = static_cast<char>(std::tolower(str[0]));
    return FunctionVal(str);
}

FunctionVal upper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
    return FunctionVal(str);
}

FunctionVal lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return FunctionVal(str);
}

FunctionVal subjective(const std::string& gender) {
    int index = genderToInt(gender);
    if (index < 0) {
        return FunctionVal();
    }
    return FunctionVal(g_SUBJECTIVE[index]);
}

FunctionVal objective(const std::string& gender) {
    int index = genderToInt(gender);
    if (index < 0) {
        return FunctionVal();
    }
    return FunctionVal(g_OBJECTIVE[index]);
}

FunctionVal possessive(const std::string& gender) {
    int index = genderToInt(gender);
    if (index < 0) {
        return FunctionVal();
    }
    return FunctionVal(g_POSSESSIVE[index]);
}

FunctionVal reflexive(const std::string& gender) {
    int index = genderToInt(gender);
    if (index < 0) {
        return FunctionVal();
    }
    return FunctionVal(g_REFLEXIVE[index]);
}

FunctionVal listConcat(const std::vector<std::vector<int>>& lists, bool isStringList) {
    size_t size = 0;
    for (const auto& list : lists) {
        size += list.size();
    }

    std::vector<int> result;
    result.reserve(size);
    for (const auto& list : lists) {
        result.insert(result.end(), list.begin(), list.end());
    }
    return FunctionVal(result, isStringList);
}

FunctionVal prev(int index, DatabaseQuery& query) {
    std::optional<std::string> prevChoice = query.getPrevChoice(index - 1);
    if(prevChoice) {
        return FunctionVal(*prevChoice);
    }
    return FunctionVal();
}

FunctionVal prevMatch(int index, const std::vector<int>& list, DatabaseQuery& query) {
    std::optional<size_t> prevChoiceIndex = query.getPrevChoiceIndex(index - 1);
    if(prevChoiceIndex) {
        if(*prevChoiceIndex < list.size()) {
            // Guaranteed to be a string list
            std::optional<std::string> str = query.getStringTable().lookup(list[*prevChoiceIndex]);
            if(str) {
                return FunctionVal(*str);
            }
        }
    }
    return FunctionVal();
}

FunctionVal pluralize(int count, std::string singular, std::string plural) {
    if (count == 1) {
        return FunctionVal(std::move(singular));
    }
    return FunctionVal(std::move(plural));
}

FunctionVal count(const std::vector<int>& list) {
    return FunctionVal(static_cast<int>(list.size()));
}

FunctionVal concat(const std::vector<std::string>& strings) {
    std::string result;
    for (const auto& str : strings) {
        result += str;
    }
    return FunctionVal(result);
}

FunctionVal add(float a, float b) {
    return FunctionVal(a + b);
}

FunctionVal sub(float a, float b) {
    return FunctionVal(a - b);
}

FunctionVal mult(float a, float b) {
    return FunctionVal(a * b);
}

FunctionVal div(float a, float b) {
    if (b == 0) {
        return FunctionVal();
    }
    return FunctionVal(a / b);
}

FunctionVal divInt(int a, int b) {
    if (b == 0) {
        return FunctionVal();
    }
    return FunctionVal(a / b);
}

FunctionVal mod(int a, int b) {
    if (b == 0) {
        return FunctionVal();
    }
    return FunctionVal(a % b);
}

FunctionVal randInt(int min, int max) {
    // TODO: Improve RNG generation
    static std::default_random_engine e;
    std::uniform_int_distribution<int> dis(min, max);
    return FunctionVal(dis(e));
}

FunctionVal num(int num) {
    std::string word = SpeechGenerator::integerToWord(num);
    if (word.empty()) {
        return FunctionVal();
    }
    return FunctionVal(word);
}

FunctionVal ord(int num) {
    std::string word = SpeechGenerator::integerToOrdinal(num);
    if (word.empty()) {
        return FunctionVal();
    }
    return FunctionVal(word);
}

FunctionVal gender(const std::string& gender, std::string neutralStr, std::string maleStr, std::string femaleStr) {
    int index = genderToInt(gender);
    if (index == 0) {
        return FunctionVal(std::move(neutralStr));
    }
    if (index == 1) {
        return FunctionVal(std::move(maleStr));
    }
    if (index == 2) {
        return FunctionVal(std::move(femaleStr));
    }
    return FunctionVal();
}

FunctionVal ifElse(const bool condition, std::string ifTrue, std::string ifFalse) {
    if (condition) {
        return FunctionVal(std::move(ifTrue));
    }
    return FunctionVal(std::move(ifFalse));
}

FunctionVal boolAnd(const bool a, const bool b) {
    return FunctionVal(a && b);
}

FunctionVal boolOr(const bool a, const bool b) {
    return FunctionVal(a || b);
}

FunctionVal boolNot(const bool a) {
    return FunctionVal(!a);
}

FunctionVal context(const std::string& table, const std::string& key, DatabaseQuery& query) {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if (contextTable == nullptr) {
        return FunctionVal();
    }
    FactType type = contextTable->getType(key);
    if (type == FactType::kString) {
        std::optional<std::string> value = contextTable->getString(key);
        if (value) {
            return FunctionVal(*value);
        }
    } else if (type == FactType::kList) {
        const std::unique_ptr<std::unordered_set<int>>& value = contextTable->getList(key);
        if (value != nullptr) {
            bool isStringList = contextTable->isStringList(key);
            std::vector<int> intList;
            intList.reserve(value->size());
            intList.insert(intList.end(), value->begin(), value->end());
            return FunctionVal(std::move(intList), isStringList);
        }
    } else if (type == FactType::kBoolean) {
        std::optional<bool> value = contextTable->getBool(key);
        if (value) {
            return FunctionVal(*value);
        }
    } else if (type == FactType::kNumber) {
        std::optional<float> value = contextTable->getFloat(key);
        if (value) {
            return FunctionVal(*value);
        }
    }
    return FunctionVal();
}

FunctionVal rawNum(int num) {
    return FunctionVal(std::to_string(num));
}

}  // namespace

void DefaultFunctionTable::initialize() {
    registerFunction("capitalize", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("uncapitalize", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("upper", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("lower", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("subjective", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("objective", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("possessive", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("reflexive", TokenType::kString, std::vector<TokenType>{TokenType::kString}, false);
    registerFunction("list_concat", TokenType::kList, std::vector<TokenType>{TokenType::kList}, true);
    registerFunction("prev", TokenType::kString, std::vector<TokenType>{TokenType::kInt}, false);
    registerFunction("prev_match", TokenType::kString, std::vector<TokenType>{TokenType::kInt, TokenType::kList},
                     false);
    registerFunction("pluralize", TokenType::kString,
                     std::vector<TokenType>{TokenType::kInt, TokenType::kString, TokenType::kString}, false);
    registerFunction("count", TokenType::kInt, std::vector<TokenType>{TokenType::kList}, false);
    registerFunction("concat", TokenType::kString, std::vector<TokenType>{TokenType::kList}, true);
    registerFunction("add", TokenType::kFloat, std::vector<TokenType>{TokenType::kFloat, TokenType::kFloat}, false);
    registerFunction("sub", TokenType::kFloat, std::vector<TokenType>{TokenType::kFloat, TokenType::kFloat}, false);
    registerFunction("mult", TokenType::kFloat, std::vector<TokenType>{TokenType::kFloat, TokenType::kFloat}, false);
    registerFunction("div", TokenType::kFloat, std::vector<TokenType>{TokenType::kFloat, TokenType::kFloat}, false);
    registerFunction("div_int", TokenType::kInt, std::vector<TokenType>{TokenType::kInt, TokenType::kInt}, false);
    registerFunction("mod", TokenType::kInt, std::vector<TokenType>{TokenType::kInt, TokenType::kInt}, false);
    registerFunction("rand_int", TokenType::kInt, std::vector<TokenType>{TokenType::kInt, TokenType::kInt}, false);
    registerFunction("num", TokenType::kString, std::vector<TokenType>{TokenType::kInt}, false);
    registerFunction("ord", TokenType::kString, std::vector<TokenType>{TokenType::kInt}, false);
    registerFunction(
        "gender", TokenType::kString,
        std::vector<TokenType>{TokenType::kString, TokenType::kString, TokenType::kString, TokenType::kString}, false);
    registerFunction("if_else", TokenType::kString,
                     std::vector<TokenType>{TokenType::kBool, TokenType::kString, TokenType::kString}, false);
    registerFunction("and", TokenType::kBool, std::vector<TokenType>{TokenType::kBool, TokenType::kBool}, false);
    registerFunction("or", TokenType::kBool, std::vector<TokenType>{TokenType::kBool, TokenType::kBool}, false);
    registerFunction("not", TokenType::kBool, std::vector<TokenType>{TokenType::kBool}, false);
    registerFunction("context", TokenType::kContext, std::vector<TokenType>{TokenType::kString, TokenType::kString},
                     false);
    registerFunction("raw_num", TokenType::kString, std::vector<TokenType>{TokenType::kInt}, false);
}

FunctionVal DefaultFunctionTable::doCall(const std::string& name, const std::vector<std::shared_ptr<SymbolToken>>& args,
                                         DatabaseQuery& query) const {
    // After many days yonder wandering the arid highlands, pondering
    // the nature of reflection--I happened upon nothing worthy of song.
    // Herein lies a much more drab tale, yet perhaps the long-winded
    // road brought me the good fortune of simplicity after all.
    if (name == "capitalize") {
        std::optional<std::string> str = argToString(args[0], query);
        if (!str) {
            return FunctionVal();
        }
        return capitalize(*str);
    }
    if (name == "uncapitalize") {
        std::optional<std::string> str = argToString(args[0], query);
        if (!str) {
            return FunctionVal();
        }
        return uncapitalize(*str);
    }
    if (name == "upper") {
        std::optional<std::string> str = argToString(args[0], query);
        if (!str) {
            return FunctionVal();
        }
        return upper(*str);
    }
    if (name == "lower") {
        std::optional<std::string> str = argToString(args[0], query);
        if (!str) {
            return FunctionVal();
        }
        return lower(*str);
    }
    if (name == "subjective") {
        std::optional<std::string> gender = argToString(args[0], query);
        if (!gender) {
            return FunctionVal();
        }
        return subjective(*gender);
    }
    if (name == "objective") {
        std::optional<std::string> gender = argToString(args[0], query);
        if (!gender) {
            return FunctionVal();
        }
        return objective(*gender);
    }
    if (name == "possessive") {
        std::optional<std::string> gender = argToString(args[0], query);
        if (!gender) {
            return FunctionVal();
        }
        return possessive(*gender);
    }
    if (name == "reflexive") {
        std::optional<std::string> gender = argToString(args[0], query);
        if (!gender) {
            return FunctionVal();
        }
        return reflexive(*gender);
    }
    if (name == "list_concat") {
        std::vector<std::vector<int>> lists;
        lists.reserve(args.size());
        bool isStringList = true;
        for (const auto& arg : args) {
            std::optional<std::pair<std::vector<int>, bool>> list = argToList(arg, query);
            if (!list) {
                return FunctionVal();
            }
            lists.push_back(std::move(list->first));
            if (!list->second) {
                isStringList = false;
            }
        }
        return listConcat(lists, isStringList);
    }
    if (name == "prev") {
        std::optional<int> index = argToInt(args[0], query);
        if (!index) {
            return FunctionVal();
        }
        return prev(*index, query);
    }
    if (name == "prev_match") {
        std::optional<int> index = argToInt(args[0], query);
        std::optional<std::pair<std::vector<int>, bool>> list = argToList(args[1], query);
        if (!index || !list || !list->second) {
            return FunctionVal();
        }
        return prevMatch(*index, list->first, query);
    }
    if (name == "pluralize") {
        std::optional<int> count = argToInt(args[0], query);
        std::optional<std::string> singular = argToString(args[1], query);
        std::optional<std::string> plural = argToString(args[2], query);
        if (!count || !singular || !plural) {
            return FunctionVal();
        }
        return pluralize(*count, *singular, *plural);
    }
    if (name == "count") {
        std::optional<std::pair<std::vector<int>, bool>> list = argToList(args[1], query);
        if (!list) {
            return FunctionVal();
        }
        return count(list->first);
    }
    if (name == "concat") {
        std::vector<std::string> strings;
        strings.reserve(args.size());
        for (const auto& arg : args) {
            std::optional<std::string> str = argToString(arg, query);
            if (!str) {
                return FunctionVal();
            }
            strings.push_back(std::move(*str));
        }
        return concat(strings);
    }
    if (name == "add") {
        std::optional<float> a = argToFloat(args[0], query);
        std::optional<float> b = argToFloat(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return add(*a, *b);
    }
    if (name == "sub") {
        std::optional<float> a = argToFloat(args[0], query);
        std::optional<float> b = argToFloat(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return sub(*a, *b);
    }
    if (name == "mult") {
        std::optional<float> a = argToFloat(args[0], query);
        std::optional<float> b = argToFloat(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return mult(*a, *b);
    }
    if (name == "div") {
        std::optional<float> a = argToFloat(args[0], query);
        std::optional<float> b = argToFloat(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return div(*a, *b);
    }
    if (name == "div_int") {
        std::optional<int> a = argToInt(args[0], query);
        std::optional<int> b = argToInt(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return divInt(*a, *b);
    }
    if (name == "mod") {
        std::optional<int> a = argToInt(args[0], query);
        std::optional<int> b = argToInt(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return mod(*a, *b);
    }
    if (name == "rand_int") {
        std::optional<int> min = argToInt(args[0], query);
        std::optional<int> max = argToInt(args[1], query);
        if (!min || !max) {
            return FunctionVal();
        }
        return randInt(*min, *max);
    }
    if (name == "num") {
        std::optional<int> n = argToInt(args[0], query);
        if (!n) {
            return FunctionVal();
        }
        return num(*n);
    }
    if (name == "ord") {
        std::optional<int> n = argToInt(args[0], query);
        if (!n) {
            return FunctionVal();
        }
        return ord(*n);
    }
    if (name == "gender") {
        std::optional<std::string> genderStr = argToString(args[0], query);
        std::optional<std::string> neutralStr = argToString(args[1], query);
        std::optional<std::string> maleStr = argToString(args[2], query);
        std::optional<std::string> femaleStr = argToString(args[3], query);
        if (!genderStr || !neutralStr || !maleStr || !femaleStr) {
            return FunctionVal();
        }
        return gender(*genderStr, *neutralStr, *maleStr, *femaleStr);
    }
    if (name == "if_else") {
        std::optional<bool> condition = argToBool(args[0], query);
        std::optional<std::string> ifTrue = argToString(args[1], query);
        std::optional<std::string> ifFalse = argToString(args[2], query);
        if (!condition || !ifTrue || !ifFalse) {
            return FunctionVal();
        }
        return ifElse(*condition, *ifTrue, *ifFalse);
    }
    if (name == "and") {
        std::optional<bool> a = argToBool(args[0], query);
        std::optional<bool> b = argToBool(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return boolAnd(*a, *b);
    }
    if (name == "or") {
        std::optional<bool> a = argToBool(args[0], query);
        std::optional<bool> b = argToBool(args[1], query);
        if (!a || !b) {
            return FunctionVal();
        }
        return boolOr(*a, *b);
    }
    if (name == "not") {
        std::optional<bool> a = argToBool(args[0], query);
        if (!a) {
            return FunctionVal();
        }
        return boolNot(*a);
    }
    if (name == "context") {
        std::optional<std::string> table = argToString(args[0], query);
        std::optional<std::string> key = argToString(args[1], query);
        if (!table || !key) {
            return FunctionVal();
        }
        return context(*table, *key, query);
    }
    if (name == "raw_num") {
        std::optional<int> n = argToInt(args[0], query);
        if (!n) {
            return FunctionVal();
        }
        return rawNum(*n);
    }
    return FunctionVal();
}

}  // namespace Contextual