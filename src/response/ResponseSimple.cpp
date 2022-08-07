#include "response/ResponseSimple.h"

#include <random>

namespace Contextual {

namespace {
const std::string g_EMPTY_STRING = "";
}

ResponseSimple::ResponseSimple(std::vector<std::string> options) : m_options(std::move(options)) {}

const std::string& ResponseSimple::getRandomOption() const {
    if(m_options.empty()) {
        return g_EMPTY_STRING;
    }
    static std::default_random_engine e;
    std::uniform_int_distribution<size_t> dis(0, m_options.size());
    size_t index = dis(e);
    return m_options[index];
}

const std::vector<std::string>& ResponseSimple::getOptions() const {
    return m_options;
}

}