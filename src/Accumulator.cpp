#include <HelixSolver/Accumulator.h>
#include <iostream>

namespace HelixSolver 
{

Accumulator::Accumulator(nlohmann::json& p_config, const std::vector<Stub>& p_stubs)
    : m_config(p_config)
    , m_stubs(p_stubs)
{
    std::cout << m_config.dump(4) << std::endl;
}

} // namespace HelixSolver


template <typename T>
std::vector<T> linspace(double start, double end, double num)
{
    std::vector<T> linspaced;
    if (0 != num)
    {
        if (1 == num) 
        {
            linspaced.push_back(static_cast<T>(start));
        }
        else
        {
            double delta = (end - start) / (num - 1);
            for (auto i = 0; i < (num - 1); ++i)
            {
                linspaced.push_back(static_cast<T>(start + delta * i));
            }
            linspaced.push_back(static_cast<T>(end));
        }
    }
    return linspaced;
}
