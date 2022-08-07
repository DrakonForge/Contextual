#pragma once

#include <memory>
#include <vector>

#include "Response.h"

namespace Contextual {

class ResponseMultiple : public Response {
public:
    ResponseMultiple(std::vector<std::shared_ptr<Response>> responses);
    const std::vector<std::shared_ptr<Response>> getResponses() const;
private:
    const std::vector<std::shared_ptr<Response>> m_responses;
};

}