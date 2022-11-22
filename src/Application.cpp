#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/TrackFindingAlgorithm.h"

#include <TFile.h>
#include <TTree.h>

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
        std::string path = config["inputRootFile"];
        std::string fileType = config.contains("inputFileType") ? std::string(config["inputFileType"]) : std::string("txt");

        std::unique_ptr<TFile> file(TFile::Open(path.c_str()));
        std::unique_ptr<TTree> hitsTree(file->Get<TTree>("spacepoints"));

        float x;
        float y;
        float z;
        uint32_t layer = 0;
        uint32_t eventId;
        hitsTree->SetBranchAddress("event_id", &eventId);
        hitsTree->SetBranchAddress("x", &x);
        hitsTree->SetBranchAddress("y", &y);
        hitsTree->SetBranchAddress("z", &z);
        // hitsTree->SetBranchAddress("layer_id", &layer);
        std::map<Event::EventId, std::vector<Stub>> stubsMap;
        for(int i = 0; hitsTree->LoadTree(i) >= 0; i++)
        {
            hitsTree->GetEntry(i);
            stubsMap[eventId].push_back(Stub{x, y, z, static_cast<uint8_t>(layer)});
        }

        std::vector<std::shared_ptr<Event>> events;
        for(auto stubs : stubsMap) events.push_back(std::make_shared<Event>(stubs.first, stubs.second));

        TrackFindingAlgorithm algorithm(config);
        algorithm.runOnGpu(events);
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
