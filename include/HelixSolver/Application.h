#pragma once

#include <nlohmann/json.hpp>

#include "HelixSolver/Event.h"

namespace HelixSolver
{
    class Application
    {
    public:
        explicit Application(std::vector<std::string>& argv);

        void Run();

        ~Application();

    private:
        nlohmann::json config;

        void loadEvent(Event& event);
        void loadConfig(const std::string& configFilePath);
    };
} // HelixSolver
