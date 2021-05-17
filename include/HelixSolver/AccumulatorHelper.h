#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>

namespace HelixSolver {

    using OptionalIdxPair = std::optional<std::pair<uint32_t, uint32_t>>;

    std::vector<float> linspace(std::vector<float> &vec, float start, float end, size_t num);

    std::pair<float, float> cart2pol(float x, float y);

    uint32_t FindClosest(const std::vector<float> &vec, float value);

    OptionalIdxPair FindYRange(nlohmann::json &p_config, std::vector<float> &p_Y, float p_yLeft, float p_yRight);

    template<typename T>
    void Swap(T &first, T &second) {
        T temp = first;
        first = second;
        second = temp;
    }

} // namespace HelixSolver
