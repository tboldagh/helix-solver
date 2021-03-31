#pragma once

#include <vector>
#include <utility>
#include <functional>

#include <nlohmann/json.hpp>
#include <HelixSolver/Event.h>
namespace HelixSolver {

    using VectorIdxPair = std::vector<std::pair<uint32_t, uint32_t>>;

    class Accumulator {
    public:
        Accumulator(nlohmann::json &p_config, const Event &m_event);

        void Fill();

        VectorIdxPair GetCellsAboveThreshold(uint8_t p_threshold) const;

        void PrintMainAcc() const;

        std::pair<double, double> GetValuesOfIndexes(uint32_t x, uint32_t y) const;

    private:
        void PrepareLinspaces();

        nlohmann::json &m_config;
        const Event &m_event;

        std::vector<double> m_X;
        std::vector<double> m_Y;

        double m_dy;
        double m_dx;
        double m_dxHalf;

        std::vector<std::vector<uint8_t>> m_map;
    };

} // namespace HelixSolver