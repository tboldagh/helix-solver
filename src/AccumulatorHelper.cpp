#include <cmath>
#include "HelixSolver/AccumulatorHelper.h"

namespace HelixSolver {

    std::vector<float> linspace(std::vector<float> &vec, float start, float end, size_t num) {
        if (0 != num) {
            if (1 == num) {
                vec.push_back(start);
            } else {
                float delta = (end - start) / (num - 1);

                for (uint32_t i = 0; i < num - 1; ++i) {
                    vec.push_back(start + delta * i);
                }
                vec.push_back(end);
            }
        }

        return vec;
    }

    std::pair<float, float> cart2pol(float x, float y) {
        float r = sqrt(x * x + y * y);
        float a = atan2(y, x);

        return std::make_pair(r, a);
    }

    uint32_t FindClosest(const std::vector<float> &vec, float value) {
        auto begin = vec.begin();
        auto end = vec.end();
        auto it = std::lower_bound(begin, end, value);
        if (it == begin) return 0;
        if (it == end) return vec.size() - 1;
        auto leftIt = it - 1;
        return value - *leftIt < *it - value ? std::distance(begin, leftIt) : std::distance(vec.begin(), it);
    }

    OptionalIdxPair FindYRange(nlohmann::json &config, std::vector<float> &Y, float yLeft, float yRight) {
        float yEnd = config["y_end"].get<float>();
        float yBegin = config["y_begin"].get<float>();
        uint32_t yDpi = config["y_dpi"].get<uint32_t>();

        if (yLeft > yBegin and yLeft < yEnd and yRight > yBegin and yRight < yEnd) {
            uint32_t rightIdx = FindClosest(Y, yRight);
            uint32_t leftIdx = FindClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft > yBegin and yRight < yBegin) {
            uint32_t rightIdx = 0;
            uint32_t leftIdx = FindClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft < yBegin and yRight > yBegin) {
            uint32_t rightIdx = FindClosest(Y, yRight);
            uint32_t leftIdx = 0;
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft > yEnd and yRight < yEnd) {
            uint32_t rightIdx = FindClosest(Y, yRight);
            uint32_t leftIdx = yDpi - 1;
            return std::make_pair(leftIdx, rightIdx);
        }
        if (yLeft < yEnd and yRight > yEnd) {
            uint32_t rightIdx = yDpi - 1;;
            uint32_t leftIdx = FindClosest(Y, yLeft);
            return std::make_pair(leftIdx, rightIdx);
        }

        return {};
    }

} // namespace HelixSolver
