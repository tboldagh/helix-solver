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
        nlohmann::json config;

        std::unique_ptr<std::vector<std::shared_ptr<Event>>> loadEvents(const std::string& path) const;
        void runOnCpu() const;

        static ComputingWorker::Platform getPlatformFromString(const std::string& platformStr);
        static std::unique_ptr<std::vector<std::shared_ptr<Event>>> loadEventsFromSpacepointsRootFile(const std::string& path);

        void loadConfig(const std::string& configFilePath);
    };
} // HelixSolver
