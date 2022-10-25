#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>

#include <CL/sycl.hpp>

namespace HelixSolver {

    using OptionalIdxPair = std::optional<std::pair<uint32_t, uint32_t>>;

    void linspace(std::vector<float> &vec, float start, float end, size_t num);
    SYCL_EXTERNAL void linspace(float* array, float start, float end, size_t numPoints);
    SYCL_EXTERNAL inline float linspaceElement(float start, float end, size_t numPoints, size_t index)
    {
        return start + (end - start) * index / (numPoints - 1);
    }

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
