#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver {

    Application::Application(std::vector<std::string> &p_argv)
            : m_argv(p_argv) {
        if (m_argv.size() < 2) {
            std::cerr << "You must pass configuration file location as program arg!" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ifstream l_configFile(m_argv[1]);
        l_configFile >> m_config;
    }

    int Application::Run() {
        m_event.LoadFromFile(m_config["inputFile"]);
        m_event.BuildStubsFunctions(m_config);
        TrackFindingAlgorithm l_algorithm(m_config, m_event);
        l_algorithm.Run();
        return 0;
    }

    Application::~Application() {
    }

} // HelixSolver
