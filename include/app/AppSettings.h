#pragma once

#include <optional>
#include <string>

struct AppSettings {
    bool debug{false};
    bool tokenize{false};
    bool showPriority{false};
    bool print{true};
    bool simple{false};
    std::optional<std::string> operation;
    std::optional<std::string> testFile;
    std::optional<std::string> group;
    std::optional<std::string> category;
    std::string queryType = "best";
    std::string inputDir = "./generated/compiled";
    int count = 10;
};