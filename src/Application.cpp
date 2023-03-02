#include <iostream>
#include <fstream>

#include "HelixSolver/Application.h"
#include "HelixSolver/ComputingManager.h"
#include "HelixSolver/Debug.h"

#include <TFile.h>
#include <TTree.h>
#include <thread>

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

    void Application::run()
    {
        ComputingWorker::Platform platform = getPlatformFromString(config["platform"]);
        switch (platform)
        {
            case ComputingWorker::Platform::CPU:
                runOnCpu();
                break;
            case ComputingWorker::Platform::CPU_NO_SYCL:
                runOnCpu();
                break;
            case ComputingWorker::Platform::GPU:
                runOnGpu();
                break;
            default:
                return;
        }
    }

    void Application::runOnCpu() const
    {
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> events = loadEvents(config["inputFile"]);

        #ifdef MULTIPLY_EVENTS
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> newEvents = std::make_unique<std::vector<std::shared_ptr<Event>>>();

        for(unsigned int i = 0; i < config["multiplyEvents"]; i++)
        {
            for(auto event : *events)
            {
                newEvents->push_back(std::shared_ptr<Event>(new Event(*event)));
            }
        }

        events.swap(newEvents);
        #endif

        ComputingManager computingManager(getPlatformFromString(config["platform"]), config["cpuEventBuffers"], config["cpuComputingWorkers"]);

        auto executionTimeStart = std::chrono::high_resolution_clock::now();

        for(std::shared_ptr<Event>& event : *events)
        {
            while(!computingManager.addEvent(event))
            {
                computingManager.waitForWaitingWorker();
                computingManager.update();
            }
        }
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> eventsAndSolutions = computingManager.transferSolutions();

        auto executionTimeEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(executionTimeEnd - executionTimeStart).count();
        INFO( "Computing "<<events->size()<<" events took "<<elapsedTime<<" microseconds" );

        printEventsAndSolutionsToFile(eventsAndSolutions, config["outputFile"].get<std::string>());
    }

    void Application::runOnGpu() const
    {
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> events = loadEvents(config["inputFile"]);

        #ifdef MULTIPLY_EVENTS
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> newEvents = std::make_unique<std::vector<std::shared_ptr<Event>>>();

        for(unsigned int i = 0; i < config["multiplyEvents"]; i++)
        {
            for(auto event : *events)
            {
                newEvents->push_back(std::shared_ptr<Event>(new Event(*event)));
            }
        }

        events.swap(newEvents);
        #endif

        ComputingManager computingManager(ComputingWorker::Platform::GPU, config["gpuEventBuffers"], config["gpuComputingWorkers"]);

        auto executionTimeStart = std::chrono::high_resolution_clock::now();

        for(std::shared_ptr<Event>& event : *events)
        {
            while(!computingManager.addEvent(event)) computingManager.update();
        }
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> eventsAndSolutions = computingManager.transferSolutions();

        auto executionTimeEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(executionTimeEnd - executionTimeStart).count();
        INFO( "Computing "<<events->size()<<" events took "<<elapsedTime<<" microseconds" );

        printEventsAndSolutionsToFile(eventsAndSolutions, config["outputFile"].get<std::string>());
    }

    ComputingWorker::Platform Application::getPlatformFromString(const std::string& platformStr)
    {
        if(platformStr == "cpu_no_sycl") return ComputingWorker::Platform::CPU_NO_SYCL;
        if(platformStr == "cpu") return ComputingWorker::Platform::CPU;
        else if(platformStr == "gpu") return ComputingWorker::Platform::GPU;
        else if(platformStr == "FPGA") return ComputingWorker::Platform::FPGA;
        else if(platformStr == "FPGA_EMULATOR") return ComputingWorker::Platform::FPGA_EMULATOR;

        return ComputingWorker::Platform::BAD_PLATFORM;
    }

    std::unique_ptr<std::vector<std::shared_ptr<Event>>> Application::loadEvents(const std::string& path) const
    {
        if(config["inputFileType"] == "root_spacepoints") return loadEventsFromSpacepointsRootFile(path);
        else
        {
            // * Reading other file types are not implemented yet.
            return std::unique_ptr<std::vector<std::shared_ptr<Event>>>();
        }
    }

    std::unique_ptr<std::vector<std::shared_ptr<Event>>> Application::loadEventsFromSpacepointsRootFile(const std::string& path)
    {
        std::unique_ptr<TFile> file(TFile::Open(path.c_str()));
        INFO( "... Input " << ( file != nullptr  ? "Opened " : "Failed to open ") << path );

        std::unique_ptr<TTree> hitsTree(file->Get<TTree>("spacepoints"));
        INFO( "... Input tree " << ( hitsTree != nullptr  ? "ok " : "absent ") << path );

        uint32_t eventId;
        float x;
        float y;
        float z;
        uint8_t layer = 0;

        hitsTree->SetBranchAddress("event_id", &eventId);
        hitsTree->SetBranchAddress("x", &x);
        hitsTree->SetBranchAddress("y", &y);
        hitsTree->SetBranchAddress("z", &z);

        std::map<Event::EventId, std::unique_ptr<std::vector<Stub>>> stubs;

        for(int i = 0; hitsTree->LoadTree(i) >= 0; i++)
        {
            hitsTree->GetEntry(i);
            if(eventId == 1){       // to analyse only a single event
                stubs.try_emplace(eventId, std::make_unique<std::vector<Stub>>());
                stubs[eventId]->push_back(Stub{x, y, z, layer});
            }
        }

        std::unique_ptr<std::vector<std::shared_ptr<Event>>> events = std::make_unique<std::vector<std::shared_ptr<Event>>>();
        for(auto& idAndStubs : stubs)
            events->push_back(std::make_shared<Event>(idAndStubs.first, std::move(idAndStubs.second)));
        return events;
    }

    void Application::printEventsAndSolutionsToFile(const std::unique_ptr<std::vector<ComputingWorker::EventSoutionsPair>>& eventsAndSolutions, const std::string& path)
    {
        std::ofstream outputFile(path);
        for (const auto& eventAndSolution : *eventsAndSolutions)
        {
            outputFile << "EventId: " << eventAndSolution.first->getId() << "\n";

            for (const SolutionCircle& solution : *eventAndSolution.second)
            {
                if (solution.isValid) outputFile << "\t" << solution.r << "\t" << solution.phi << std::endl;
            }

            outputFile << "\n";
        }
    }

    void Application::loadConfig(const std::string& configFilePath)
    {
        std::ifstream configFile(configFilePath);
        configFile >> config;
    }
} // HelixSolver