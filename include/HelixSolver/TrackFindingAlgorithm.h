#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "HelixSolver/Stub.h"
#include "HelixSolver/KernelExecutionContainer.h"

namespace HelixSolver {

    class TrackFindingAlgorithm {
    public:
        TrackFindingAlgorithm(nlohmann::json &config, const Event &event);

        void Run();

    private:
        nlohmann::json &config;
        const double B{0};
        const Event &event;
        KernelExecutionContainer kec;
    };

} // HelixSolver