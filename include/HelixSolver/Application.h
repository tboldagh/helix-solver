#pragma once

#include <nlohmann/json.hpp>

#include "HelixSolver/Event.h"

namespace HelixSolver {

    class Application {
    public:
        explicit Application(std::vector<std::string> &p_argv);

        int Run();

        ~Application();

    private:
        Event m_event;
        nlohmann::json m_config;
        std::vector<std::string> &m_argv;

        void load_event();
    };

} // HelixSolver
