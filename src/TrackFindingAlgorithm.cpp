#include <fstream>
#include <boost/math/constants/constants.hpp>

#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver {

    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json &p_config, const Event& p_event)
            : m_config(p_config), m_B(p_config["B"].get<double>()), m_event(p_event),
              m_kec(m_config["main_accumulator_config"], m_event) {
    }

    void TrackFindingAlgorithm::Run() {
        m_kec.FillOnDevice();
        m_kec.PrintMainAcc();
        std::ofstream out(m_config["outputFile"].get<std::string>());
        out << std::setprecision(64);
        for (const auto &solution : m_kec.GetSolution()) {
            if (solution.isValid) {
                out << solution.r << " " << solution.phi << std::endl;
            } 
        }
    }

} // namespace HelixSolver
