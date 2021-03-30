#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include <HelixSolver/Stub.h>
#include <HelixSolver/Accumulator.h>

namespace HelixSolver {

    class TrackFindingAlgorithm {
    public:
        TrackFindingAlgorithm(nlohmann::json &p_config, const std::vector<Stub> &p_stubs);

        void Run();

    private:
        nlohmann::json &m_config;
        const double m_B{0};
        const std::vector<Stub> &m_stubs;
        Accumulator m_firstStageAcc;
    };

} // HelixSolver
