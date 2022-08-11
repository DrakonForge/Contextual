#include "Criterion.h"

namespace Contextual {

int Criterion::count = 0;

int Criterion::getCount() {
    return Criterion::count;
}

Criterion::Criterion() {
    ++count;
}

Criterion::~Criterion() {
    --count;
}

}  // namespace Contextual