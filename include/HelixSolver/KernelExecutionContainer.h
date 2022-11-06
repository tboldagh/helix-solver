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
        enum class Platform
        {
            BAD_PLATFORM,
            CPU,
            GPU,
            FPGA,
            FPGA_EMULATOR
        };

        KernelExecutionContainer(nlohmann::json& config, Event& event);

        void fillOnDevice();
        void printInfo(const std::unique_ptr<sycl::queue>& queue) const;
        VectorIdxPair getCellsAboveThreshold(uint8_t threshold) const;
        void printMainAcc() const;
        // TODO: make inline
        const std::array<SolutionCircle, ACC_SIZE>& getSolution() const;

    private:
        sycl::queue* getQueue();

        nlohmann::json& config;
        Event& event;

        std::array<SolutionCircle, ACC_SIZE> map;
        Platform platform;
    };

} // namespace HelixSolver
