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
        TrackFindingAlgorithm(nlohmann::json& config, Event& event);

        void run();

    private:
        nlohmann::json& config;
        // TODO: rename
        const double B{0};
        KernelExecutionContainer kernelExecutionContainer;
    };
} // HelixSolver