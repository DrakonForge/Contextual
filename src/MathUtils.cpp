#include "MathUtils.h"

#include <random>
#include <chrono>

namespace Contextual::MathUtils {

namespace {

std::random_device rd;
std::mt19937 engine(rd());

}

int randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine);
}

size_t randUInt(size_t min, size_t max) {
    std::uniform_int_distribution<size_t> dist(min, max);
    return dist(engine);
}

float randFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}

}