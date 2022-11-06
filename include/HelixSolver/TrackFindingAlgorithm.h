#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "HelixSolver/Stub.h"
#include "HelixSolver/KernelExecutionContainer.h"

namespace HelixSolver
{
    class TrackFindingAlgorithm
    {
    public:
        TrackFindingAlgorithm(nlohmann::json& config, std::shared_ptr<Event> event);

        void run();
        void runOnGpu();

    private:
        nlohmann::json& config;
        // TODO: rename
        const double B{0};
        std::shared_ptr<Event> event;
        KernelExecutionContainer kernelExecutionContainer;
    };
} // HelixSolver