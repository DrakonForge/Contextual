#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/MessageOnlyFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

const std::string g_OP_QUERY = "query";
const std::string g_OP_TEST = "test";

const std::string g_QUERY_BEST = "best";
const std::string g_QUERY_ALL = "all";

struct Flags {
    bool debug{false};
    bool tokenize{false};
    bool showPriority{false};
    bool print{true};
    bool simple{false};
    std::optional<std::string> operation;
    std::optional<std::string> testFile;
    std::optional<std::string> group;
    std::optional<std::string> category;
    std::string queryType = g_QUERY_BEST;
    std::string inputDir = "./generated/compiled";
    int count = 10;
};

const std::unordered_map<std::string, std::function<void(Flags&)>> NoArgHandles = {
    {"--debug", [](Flags& f) { f.debug = true; }},
    {"--noprint", [](Flags& f) { f.print = false; }},
    {"--tokenize", [](Flags& f) { f.tokenize = true; }},
    {"--priority", [](Flags& f) { f.showPriority = true; }},
    {"--simple", [](Flags& f) { f.simple = true; }}};

const std::unordered_map<std::string, std::function<void(Flags&, const std::string&)>> OneArgHandles = {
    {"--input", [](Flags& f, const std::string& arg) { f.inputDir = arg; }},
    {"--group", [](Flags& f, const std::string& arg) { f.group = arg; }},
    {"--category", [](Flags& f, const std::string& arg) { f.category = arg; }},
    {"--query", [](Flags& f, const std::string& arg) { f.queryType = arg; }},
    {"--count", [](Flags& f, const std::string& arg) { f.count = std::stoi(arg); }}};

Flags parseFlags(int argc, const char* argv[]) {
    Flags flags;

    for (int i = 1; i < argc; ++i) {
        std::string option = argv[i];

        auto noArg = NoArgHandles.find(option);
        if (noArg != NoArgHandles.end()) {
            noArg->second(flags);
            continue;
        }

        auto oneArg = OneArgHandles.find(option);
        if (oneArg != OneArgHandles.end()) {
            if (++i < argc) {
                oneArg->second(flags, argv[i]);
            } else {
                std::cerr << "Missing parameter after " << option << std::endl;
            }
            continue;
        }

        // Positional arguments
        if (!flags.operation) {
            flags.operation = argv[i];
        } else if (!flags.testFile) {
            flags.testFile = argv[i];
        } else {
            std::cerr << "Unrecognized command line option " << option << std::endl;
        }
    }

    return flags;
}

void printUsage(const std::string& name) {
    PLOG_INFO << "Usage: " << name << " <operation>\n\n"
              << g_OP_QUERY
              << " <query_json> [path_to_rule_database]\n"
                 "\tQueries the rule database with a group, category, and context.\n";
}

void doQuery(Flags& flags) {
    PLOG_INFO << "QUERY";
}

void doTest(Flags& flags) {
    PLOG_INFO << "TEST";
}

// https://blog.nickelp.ro/posts/min-guide-to-cli/
int main(int argc, const char* argv[]) {
    Flags flags = parseFlags(argc, argv);

    // Initializer
    static plog::ColorConsoleAppender<plog::MessageOnlyFormatter> consoleAppender;
    if (flags.debug) {
        plog::init(plog::verbose, &consoleAppender);
    } else {
        plog::init(plog::info, &consoleAppender);
    }

    // Help menu
    if (!flags.operation) {
        printUsage(argv[0]);
        return -1;
    }

    if (*flags.operation == g_OP_QUERY) {
        doQuery(flags);
    } else if (*flags.operation == g_OP_TEST) {
        doTest(flags);
    } else {
        PLOG_ERROR << "Unrecognized operation \"" << *flags.operation << "\"";
        printUsage(argv[0]);
    }
    return 0;
}