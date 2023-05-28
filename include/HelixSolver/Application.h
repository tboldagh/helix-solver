#pragma once

#include <nlohmann/json.hpp>

#include "HelixSolver/Event.h"
#include "HelixSolver/ComputingWorker.h"

namespace HelixSolver
{
    class Application
    {
    public:
        explicit Application(std::vector<std::string>& argv);

        void run();

    private:

        std::unique_ptr<std::vector<std::shared_ptr<Event>>> loadEvents(const std::string& path) const;
        void runOnCpu() const;
        void runOnGpu() const;

        static ComputingWorker::Platform getPlatformFromString(const std::string& platformStr);
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> loadEventsFromSpacepointsRootFile(const std::string& path) const;
        static void saveSolutionsInRootFile(const std::unique_ptr<std::vector<ComputingWorker::EventSoutionsPair>>& eventsAndSolutions, const std::string& path);
        std::function<bool(float, float, float)> selector () const;

        void loadConfig(const std::string& configFilePath);
    };
} // HelixSolver
