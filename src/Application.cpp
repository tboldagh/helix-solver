#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/TrackFindingAlgorithm.h"

namespace HelixSolver
{
    Application::Application(std::vector<std::string>& argv)
    {
        if (argv.size() < 2)
        {
            std::cerr << "You must pass configuration file location as program arg!" << std::endl;
            exit(EXIT_FAILURE);
        }

        loadConfig(argv[1]);
    }

    void Application::Run()
    {
        Event event;
        loadEvent(event);
        // TODO: Move calculating stub functions to accelerator device
        event.buildStubsFunctions();
        TrackFindingAlgorithm algorithm(config, event);
        algorithm.run();
    }

    Application::~Application()
    {

    }

    void Application::loadEvent(Event& event)
    {
        std::string path = config["inputFile"];
        std::string fileType = config.contains("inputFileType") ? std::string(config["inputFileType"]) : std::string("txt");
        event.loadFromFile(path, fileType);
    }

    void Application::loadConfig(const std::string& configFilePath)
    {
        std::ifstream configFile(configFilePath);
        configFile >> config;   
    }
} // HelixSolver
