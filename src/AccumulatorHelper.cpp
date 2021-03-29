#include <cmath>
#include <HelixSolver/AccumulatorHelper.h>

namespace HelixSolver
{

std::vector<double> linspace(std::vector<double>& vec, double start, double end, size_t num)
{
    if (0 != num)
    {
        if (1 == num)
        {
            vec.push_back(start);
        }
        else
        {
            double delta = (end - start) / (num - 1);

            for (auto i = 0; i < (num - 1); ++i)
            {
                vec.push_back(start + delta * i);
            }
            vec.push_back(end);
        }
    }

    return vec;
}

std::pair<double, double> cart2pol(double x, double y)
{
    double r = sqrt(x*x + y*y);
    double a = atan2(y, x);

    return std::make_pair(r, a);
}

uint32_t FindClosest(const std::vector<double>& vec, double value)
{
    auto begin = vec.begin();
    auto end = vec.end();
    auto it = std::lower_bound(begin, end, value);
    if (it == begin) return 0;
    if (it == end) return vec.size() - 1;
    auto leftIt = it - 1;
    return value - *leftIt < *it - value ? std::distance(begin, leftIt) : std::distance(vec.begin(), it);
}

OptionalIdxPair FindYRange(nlohmann::json& p_config, std::vector<double>& p_Y, double p_yLeft, double p_yRight)
{
    double yEnd = p_config["y_end"].get<double>();
    double yBegin = p_config["y_begin"].get<double>();
    uint32_t yDpi= p_config["y_dpi"].get<uint32_t>();

    if ((p_yLeft > yBegin) and (p_yLeft < yEnd) and (p_yRight > yBegin) and (p_yRight < yEnd))
    {
        uint32_t rightIdx = FindClosest(p_Y, p_yRight);
        uint32_t leftIdx = FindClosest(p_Y, p_yLeft);
        return std::make_pair(leftIdx, rightIdx);
    }
    // TODO: finish all cases
    return {};
}

} // namespace HelixSolver
