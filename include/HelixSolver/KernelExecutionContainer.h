#pragma once

#include <vector>
#include <array>
#include <utility>
#include <functional>

#include <nlohmann/json.hpp>
#include "HelixSolver/Event.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/SolutionCircle.h"
#include <sycl/ext/intel/fpga_extensions.hpp>

namespace HelixSolver
{
    using VectorIdxPair = std::vector<std::pair<uint32_t, uint32_t>>;

    class KernelExecutionContainer
    {
    public:
        KernelExecutionContainer(nlohmann::json& config, Event& event);

        void fill();
        void fillOnDevice();
        void printInfo(const std::unique_ptr<sycl::queue>& queue) const;
        VectorIdxPair getCellsAboveThreshold(uint8_t threshold) const;
        void printMainAcc() const;
        // TODO: make inline
        const std::array<SolutionCircle, ACC_SIZE>& getSolution() const;
        // TODO: make inline
        std::pair<double, double> getValuesOfIndexes(uint32_t x, uint32_t y) const;

    private:
        void prepareLinspaces();
        sycl::queue* getFpgaQueue();

        nlohmann::json& config;
        Event& event;

        std::vector<float> xs;
        std::vector<float> ys;

        double dx;
        double dy;
        double dxHalf;

        std::array<SolutionCircle, ACC_SIZE> map;
    };

} // namespace HelixSolver
