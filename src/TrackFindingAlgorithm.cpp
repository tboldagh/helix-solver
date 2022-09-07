#include <fstream>
#include <boost/math/constants/constants.hpp>

#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver {

    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json &p_config, const Event& p_event)
            : config(p_config), B(p_config["B"].get<double>()), event(p_event),
              kec(config["main_accumulator_config"], event) {
    }

    void TrackFindingAlgorithm::Run() {
        kec.FillOnDevice();
        kec.PrintMainAcc();
        std::ofstream out(config["outputFile"].get<std::string>());
        out << std::setprecision(64);
        for (const auto &solution : kec.GetSolution()) {
            if (solution.isValid) {
                out << solution.r << " " << solution.phi << std::endl;
            } 
        }
    }

} // namespace HelixSolver
