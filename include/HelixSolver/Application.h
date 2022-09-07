#pragma once

#include <nlohmann/json.hpp>

#include "HelixSolver/Event.h"

namespace HelixSolver {

    class Application {
    public:
        explicit Application(std::vector<std::string> &argv);

        int Run();

        ~Application();

    private:
        Event event;
        nlohmann::json config;
        std::vector<std::string> &argv;

        void load_event();
    };

} // HelixSolver
