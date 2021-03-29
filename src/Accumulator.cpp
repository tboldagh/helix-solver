#include <iostream>
#include <HelixSolver/Accumulator.h>
#include <HelixSolver/AccumulatorHelper.h>

namespace HelixSolver 
{

Accumulator::Accumulator(nlohmann::json& p_config, const std::vector<Stub>& p_stubs)
    : m_config(p_config)
    , m_stubs(p_stubs)
{
    PrepareLinspaces();

    m_dx = m_X[1] - m_X[0];
    m_dxHalf = m_dx / 2;

    m_dy = m_Y[1] - m_Y[0];

    m_map.resize(m_config["y_dpi"]);
    for (auto& row : m_map)
    {
        row.resize(m_config["x_dpi"]);
    }
}

void Accumulator::Fill()
{
    for (auto& stub : m_stubs)
    {
        const auto [r, phi] = cart2pol(stub.x, stub.y);
        auto fun = [r = r, phi = phi](double x){ return -r * x + phi; };

        for (uint32_t idx = 0; idx < m_X.size(); ++idx)
        {
            double x = m_X[idx];
            double xLeft = x - m_dxHalf;
            double xRight = x + m_dxHalf;
            double yLeft = fun(xLeft);
            double yRight = fun(xRight);
            OptionalIdxPair yRange= FindYRange(m_config, m_Y, yLeft, yRight);
            // TODO: finish filling accumulator
        }
    }
}


void Accumulator::PrepareLinspaces()
{
    linspace(m_X,
             m_config["x_begin"].get<double>(),
             m_config["x_end"].get<double>(),
             m_config["x_dpi"].get<size_t>());

    linspace(m_Y,
             m_config["y_begin"].get<double>(),
             m_config["y_end"].get<double>(),
             m_config["y_dpi"].get<size_t>());
}

} // namespace HelixSolver
