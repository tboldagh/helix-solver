#include <cmath>
#include "HelixSolver/AccumulatorHelper.h"

namespace HelixSolver
{
    void linspace(std::vector<float>& vec, float start, float end, size_t numPoints)
    {
        float delta = end - start;
        vec.push_back(start);
        for (uint32_t i = 1; i < numPoints; ++i)
        {
            vec.push_back(start + delta * i / (numPoints - 1));
        }
    }

    std::pair<float, float> cart2pol(float x, float y)
    {
        float r = sqrt(x * x + y * y);
        float a = atan2(y, x);

        return std::make_pair(r, a);
    }

    uint32_t findClosest(const std::vector<float> &vec, float value)
    {
        auto begin = vec.begin();
        auto end = vec.end();
        auto it = std::lower_bound(begin, end, value);
        if (it == begin) return 0;
        if (it == end) return vec.size() - 1;
        auto leftIt = it - 1;
        return value - *leftIt < *it - value ? std::distance(begin, leftIt) : std::distance(vec.begin(), it);
    }

    OptionalIdxPair findYRange(nlohmann::json &config, std::vector<float> &Y, float yLeft, float yRight)
    {
        float yEnd = config["y_end"].get<float>();
        float yBegin = config["y_begin"].get<float>();
        uint32_t yDpi = config["y_dpi"].get<uint32_t>();

        if (yLeft > yBegin and yLeft < yEnd and yRight > yBegin and yRight < yEnd) {
            uint32_t rightIdx = findClosest(Y, yRight);
            uint32_t leftIdx = findClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft > yBegin and yRight < yBegin) {
            uint32_t rightIdx = 0;
            uint32_t leftIdx = findClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft < yBegin and yRight > yBegin) {
            uint32_t rightIdx = findClosest(Y, yRight);
            uint32_t leftIdx = 0;
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft > yEnd and yRight < yEnd) {
            uint32_t rightIdx = findClosest(Y, yRight);
            uint32_t leftIdx = yDpi - 1;
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft < yEnd and yRight > yEnd) {
            uint32_t rightIdx = yDpi - 1;;
            uint32_t leftIdx = findClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }

        return {};
    }

} // namespace HelixSolver
