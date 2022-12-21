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

    void linspace(float* array, float start, float end, size_t numPoints)
    {
        float delta = end - start;
        for (uint32_t i = 0; i < numPoints; ++i)
        {
            array[i] = start + delta * i / (numPoints - 1);
        }
    }

    std::pair<float, float> cart2pol(float x, float y)
    {
        float r = sqrt(x * x + y * y);
        float a = atan2(y, x);

        return std::make_pair(r, a);
    }
} // namespace HelixSolver
