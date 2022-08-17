#include "ResponseSimple.h"

#include "MathUtils.h"

namespace Contextual {

namespace {
const std::string g_EMPTY_STRING = "";
}

ResponseSimple::ResponseSimple(std::vector<std::string> options) : m_options(std::move(options)) {}

const std::string& ResponseSimple::getRandomOption() const {
    if (m_options.empty()) {
        return g_EMPTY_STRING;
    }
    size_t index = MathUtils::randUInt(0, m_options.size() - 1);
    return m_options[index];
}

const std::vector<std::string>& ResponseSimple::getOptions() const {
    return m_options;
}

ResponseType ResponseSimple::getType() const {
    return ResponseType::kSimple;
}

}  // namespace Contextual