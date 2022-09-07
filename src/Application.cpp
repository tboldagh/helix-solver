#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver {

    Application::Application(std::vector<std::string> &argv)
    : argv(argv)
    {
        if (argv.size() < 2)
        {
            std::cerr << "You must pass configuration file location as program arg!" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ifstream l_configFile(argv[1]);
        l_configFile >> config;
    }

    int Application::Run()
    {
        load_event();
        event.BuildStubsFunctions(config);
        TrackFindingAlgorithm l_algorithm(config, event);
        l_algorithm.Run();
        return 0;
    }

    Application::~Application() {
    }

    void Application::load_event()
    {
        if(config.contains("inputFileType"))
        {
            event.LoadFromFile(config["inputFile"]);
            if(config["inputFileType"] == std::string("root"))
            {
                event.loadFromRootFile(config["inputRootFile"]);
            }
            else if(config["inputFileType"] == std::string("txt"))
            {
                event.LoadFromFile(config["inputFile"]);
            }
        }
        else
        {
            event.LoadFromFile(config["inputFile"]);
        }
    }

} // HelixSolver
