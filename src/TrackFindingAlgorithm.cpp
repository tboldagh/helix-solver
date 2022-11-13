#include <fstream>
#include <boost/math/constants/constants.hpp>

#include "HelixSolver/TrackFindingAlgorithm.h"
#include "HelixSolver/ComputingManager.h"

namespace HelixSolver
{
    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json& config, std::shared_ptr<Event> event)
    : config(config)
    , B(config["B"].get<double>())
    , event(event) {}

    void TrackFindingAlgorithm::runOnGpu()
    {
        ComputingManager computingManager(ComputingManager::Platform::CPU, 1, 1);

        computingManager.addEvent(event);
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> solutionsP = computingManager.transferSolutions();

        // * For testing purposes only
        std::unique_ptr<std::vector<SolutionCircle>> solutions = std::move((*solutionsP)[0].second);
        std::ofstream out(config["outputFile"].get<std::string>());
        for (const SolutionCircle& solution : *solutions)
        {
            if (solution.isValid) out << solution.r << " " << solution.phi << std::endl;
        }
    }

} // namespace HelixSolver
