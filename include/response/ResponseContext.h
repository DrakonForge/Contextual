#pragma ResponseContext

#include <string>

#include "DatabaseQuery.h"
#include "Response.h"

namespace Contextual {

class ResponseContext : public Response {
public:
    ResponseContext(std::string table, std::string key);
    virtual void execute(DatabaseQuery& query) = 0;
    [[nodiscard]] ResponseType getType() const override;

protected:
    std::string m_table;
    std::string m_key;
};

}  // namespace Contextual