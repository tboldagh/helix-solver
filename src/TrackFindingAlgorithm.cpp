#include <fstream>
#include <boost/math/constants/constants.hpp>

#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver
{
    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json& config, Event& event)
    : config(config)
    , B(config["B"].get<double>())
    , event(event)
    , kernelExecutionContainer(config["main_accumulator_config"], event) {}

    void TrackFindingAlgorithm::run()
    {
        kernelExecutionContainer.fillOnDevice();
        kernelExecutionContainer.printMainAcc();
        std::ofstream out(config["outputFile"].get<std::string>());
        out << std::setprecision(64);
        for (const auto &solution : kernelExecutionContainer.getSolution())
        {
            if (solution.isValid) out << solution.r << " " << solution.phi << std::endl;
        }
    }
} // namespace HelixSolver
