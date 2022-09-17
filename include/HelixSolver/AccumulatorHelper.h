#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>

namespace HelixSolver {

    using OptionalIdxPair = std::optional<std::pair<uint32_t, uint32_t>>;

    std::vector<float> linspace(std::vector<float> &vec, float start, float end, size_t num);

    std::pair<float, float> cart2pol(float x, float y);

    uint32_t findClosest(const std::vector<float> &vec, float value);

    OptionalIdxPair findYRange(nlohmann::json &config, std::vector<float> &Y, float yLeft, float yRight);

    template<typename T>
    void Swap(T &first, T &second) {
        T temp = first;
        first = second;
        second = temp;
    }

} // namespace HelixSolver
