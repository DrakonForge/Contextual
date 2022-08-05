#pragma once

#include <string>

#include "DatabaseQuery.h"

namespace Contextual {

class Criterion {
public:
    static int getCount();
    [[nodiscard]] virtual bool evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const = 0;
    [[nodiscard]] virtual int getPriority() const = 0;
protected:
    Criterion();
    ~Criterion();
private:
    static int count;
};

}