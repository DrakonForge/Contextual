#include "ResponseMultiple.h"

namespace Contextual {

ResponseMultiple::ResponseMultiple(std::vector<std::shared_ptr<Response>> responses)
    : m_responses(std::move(responses)) {}

std::vector<std::shared_ptr<Response>> ResponseMultiple::getResponses() const {
    return m_responses;
}

ResponseType ResponseMultiple::getType() const {
    return ResponseType::kMultiple;
}

}  // namespace Contextual