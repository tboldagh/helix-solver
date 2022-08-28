#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver {

    Application::Application(std::vector<std::string> &p_argv)
    : m_argv(p_argv)
    {
        if (m_argv.size() < 2)
        {
            std::cerr << "You must pass configuration file location as program arg!" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ifstream l_configFile(m_argv[1]);
        l_configFile >> m_config;
    }

    int Application::Run()
    {
        load_event();
        m_event.BuildStubsFunctions(m_config);
        TrackFindingAlgorithm l_algorithm(m_config, m_event);
        l_algorithm.Run();
        return 0;
    }

    Application::~Application() {
    }

    void Application::load_event()
    {
        if(m_config.contains("inputFileType"))
        {
            m_event.LoadFromFile(m_config["inputFile"]);
            if(m_config["inputFileType"] == std::string("root"))
            {
                m_event.loadFromRootFile(m_config["inputRootFile"]);
            }
            else if(m_config["inputFileType"] == std::string("txt"))
            {
                m_event.LoadFromFile(m_config["inputFile"]);
            }
        }
        else
        {
            m_event.LoadFromFile(m_config["inputFile"]);
        }
    }

} // HelixSolver
