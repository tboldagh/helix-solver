#pragma once

#include <vector>
#include <array>
#include <utility>
#include <functional>

#include <nlohmann/json.hpp>
#include "HelixSolver/Event.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/SolutionCircle.h"

namespace HelixSolver {

    using VectorIdxPair = std::vector<std::pair<uint32_t, uint32_t>>;

    class KernelExecutionContainer {
    public:
        KernelExecutionContainer(nlohmann::json &config, const Event &event);

        void Fill();

        void FillOnDevice();

        VectorIdxPair GetCellsAboveThreshold(uint8_t threshold) const;

        void PrintMainAcc() const;

        std::pair<double, double> GetValuesOfIndexes(uint32_t x, uint32_t y) const;

        const std::array<SolutionCircle, ACC_SIZE> &GetSolution() const;

    private:
        void PrepareLinspaces();

        nlohmann::json &config;
        const Event &event;

        std::vector<float> xs;
        std::vector<float> ys;

        double dx;
        double dy;
        double dxHalf;

        std::array<SolutionCircle, ACC_SIZE> m_map;
    };

} // namespace HelixSolver
