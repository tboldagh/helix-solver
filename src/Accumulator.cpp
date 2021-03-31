#include <iostream>
#include <HelixSolver/Accumulator.h>
#include <HelixSolver/AccumulatorHelper.h>

namespace HelixSolver {

    Accumulator::Accumulator(nlohmann::json &p_config, const Event &m_event)
            : m_config(p_config), m_event(m_event) {
        PrepareLinspaces();

        m_dx = m_X[1] - m_X[0];
        m_dxHalf = m_dx / 2;

        m_dy = m_Y[1] - m_Y[0];

        m_map.resize(m_config["y_dpi"]);
        for (auto &row : m_map) {
            row.resize(m_config["x_dpi"]);
        }
    }

    void Accumulator::Fill() {
        for (const auto& stubFunc : m_event.GetStubsFuncs()) {
            for (uint32_t i = 0; i < m_X.size(); ++i) {
                double x = m_X[i];
                double xLeft = x - m_dxHalf;
                double xRight = x + m_dxHalf;
                double yLeft = stubFunc(xLeft);
                double yRight = stubFunc(xRight);
                OptionalIdxPair yRange = FindYRange(m_config, m_Y, yLeft, yRight);
                if (!yRange.has_value())
                    continue;
                if (yRange.value().first > yRange.value().second)
                    Swap(yRange.value().first, yRange.value().second);
                for (uint32_t j = yRange.value().first; j <= yRange.value().second; ++j) {
                    m_map[j][i] += 1;
                }
            }
        }
    }

    VectorIdxPair Accumulator::GetCellsAboveThreshold(uint8_t p_threshold) const {
        VectorIdxPair cellsAboveThreshold;
        for (uint32_t i = 0; i < m_map.size(); ++i) {
            for (uint32_t j = 0; j < m_map[i].size(); ++j) {
                if (m_map[i][j] >= p_threshold)
                    cellsAboveThreshold.push_back(std::make_pair(j, i));
            }
        }
        return cellsAboveThreshold;
    }

    void Accumulator::PrepareLinspaces() {
        linspace(m_X,
                 m_config["x_begin"].get<double>(),
                 m_config["x_end"].get<double>(),
                 m_config["x_dpi"].get<size_t>());

        linspace(m_Y,
                 m_config["y_begin"].get<double>(),
                 m_config["y_end"].get<double>(),
                 m_config["y_dpi"].get<size_t>());
    }

    void Accumulator::PrintMainAcc() const {
        for (uint32_t i = 0; i < m_map.size(); ++i) {
            for (uint32_t j = 0; j < m_map[i].size(); ++j) {
                std::cout << int(m_map[i][j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    std::pair<double, double> Accumulator::GetValuesOfIndexes(uint32_t x, uint32_t y) const {
        return std::pair<double, double>(m_X[x], m_Y[y]);
    }

} // namespace HelixSolver
