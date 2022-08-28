#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "HelixSolver/Stub.h"
#include "HelixSolver/KernelExecutionContainer.h"

namespace HelixSolver {

    class TrackFindingAlgorithm {
    public:
        TrackFindingAlgorithm(nlohmann::json &p_config, const Event &p_event);

        void Run();

    private:
        nlohmann::json &m_config;
        const double m_B{0};
        const Event &m_event;
        KernelExecutionContainer m_kec;
    };

} // HelixSolver