#include "ResponseEvent.h"

namespace Contextual {

ResponseEvent::ResponseEvent(std::string name, std::vector<std::string> args)
    : m_name(std::move(name)), m_args(std::move(args)) {}

const std::string& ResponseEvent::getName() const {
    return m_name;
}

const std::vector<std::string>& ResponseEvent::getArgs() const {
    return m_args;
}

ResponseType ResponseEvent::getType() const {
    return ResponseType::kEvent;
}

}  // namespace Contextual
