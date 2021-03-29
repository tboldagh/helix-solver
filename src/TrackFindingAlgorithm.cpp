#include <iostream>
#include <HelixSolver/TrackFindingAlgorithm.h>

namespace HelixSolver
{

TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json& p_config, const std::vector<Stub>& p_stubs)
    : m_config(p_config)
    , m_stubs(p_stubs)
    , m_firstStageAcc(m_config["main_accumulator_config"], m_stubs)
{
}

void TrackFindingAlgorithm::Run()
{
    m_firstStageAcc.Fill();
}

} // namespace HelixSolver
