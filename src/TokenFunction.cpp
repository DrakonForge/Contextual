#include "TokenFunction.h"

#include <utility>

namespace Contextual {

TokenFunction::TokenFunction(std::string name,
                             std::vector<std::shared_ptr<Token>> args) : m_name(std::move(name)), m_args(std::move(args)) {

}

std::optional<std::string> TokenFunction::evaluate(const DatabaseQuery& query) const {
    // TODO
    return std::nullopt;
}


}