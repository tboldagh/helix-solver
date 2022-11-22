#include <fstream>
#include <boost/math/constants/constants.hpp>

#include "HelixSolver/TrackFindingAlgorithm.h"
#include "HelixSolver/ComputingManager.h"

namespace HelixSolver
{
    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json& config)
    : config(config)
    , B(config["B"].get<double>()) {}

    TrackFindingAlgorithm::TrackFindingAlgorithm(nlohmann::json& config, std::shared_ptr<Event> event)
    : config(config)
    , B(config["B"].get<double>())
    , event(event) {}

    void TrackFindingAlgorithm::runOnGpu()
    {
        ComputingManager computingManager(ComputingWorker::Platform::CPU, 1, 1);

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

    void TrackFindingAlgorithm::runOnGpu(std::vector<std::shared_ptr<Event>>& events)
    {
        ComputingManager computingManager(ComputingWorker::Platform::CPU, 10, 5);

        for (auto e : events)
        {
            while(!computingManager.addEvent(e)) computingManager.update();
        }
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> eventAndSolutions = computingManager.transferSolutions();

        std::ofstream out(config["outputFile"].get<std::string>());
        for (const auto& eventAndSolution : *eventAndSolutions)
        {
            out << "EventId: " << eventAndSolution.first->getId() << "\n";

            for (const SolutionCircle& solution : *eventAndSolution.second)
            {
                if (solution.isValid) out << "\t" << solution.r << "\t" << solution.phi << std::endl;
            }

            out << "\n";
        }
    }

} // namespace HelixSolver
