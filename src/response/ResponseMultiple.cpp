#include "response/ResponseMultiple.h"

namespace Contextual {

ResponseMultiple::ResponseMultiple(std::vector<std::shared_ptr<Response>> responses) : m_responses(std::move(responses)) {}

const std::vector<std::shared_ptr<Response>> ResponseMultiple::getResponses() const {
    return m_responses;
}

}