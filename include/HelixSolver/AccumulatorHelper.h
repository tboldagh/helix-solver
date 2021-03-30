#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>

namespace HelixSolver {

    using OptionalIdxPair = std::optional<std::pair<uint32_t, uint32_t>>;

    std::vector<double> linspace(std::vector<double> &vec, double start, double end, size_t num);

    std::pair<double, double> cart2pol(double x, double y);

    uint32_t FindClosest(const std::vector<double> &vec, double value);

    OptionalIdxPair FindYRange(nlohmann::json &p_config, std::vector<double> &p_Y, double p_yLeft, double p_yRight);

    template<typename T>
    void Swap(T &first, T &second) {
        T temp = first;
        first = second;
        second = temp;
    }

} // namespace HelixSolver
