#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <stdexcept>
#include <thread>

#include "HelixSolver/Application.h"
#include "HelixSolver/ComputingManager.h"
#include "Debug/Debug.h"
#include "HelixSolver/Constants.h"

nlohmann::json config;

namespace HelixSolver
{
    Application::Application(std::vector<std::string> &argv)
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

        for (unsigned int i = 0; i < config["multiplyEvents"]; i++)
        {
            for (auto event : *events)
            {
                newEvents->push_back(std::shared_ptr<Event>(new Event(*event)));
            }
        }

        events.swap(newEvents);
#endif

        ComputingManager computingManager(getPlatformFromString(config["platform"]), config["cpuEventBuffers"], config["cpuComputingWorkers"]);

        auto executionTimeStart = std::chrono::high_resolution_clock::now();

        for (std::shared_ptr<Event> &event : *events)
        {
            while (!computingManager.addEvent(event))
            {
                computingManager.waitForWaitingWorker();
                computingManager.update();
            }
        }
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> eventsAndSolutions = computingManager.transferSolutions();

        auto executionTimeEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(executionTimeEnd - executionTimeStart).count();
        auto elapsedTime_sec = round(float(elapsedTime) / 1e3) / 1e3;
        INFO("Computing " << events->size() << " events took " << elapsedTime_sec << " seconds");

        saveSolutionsInRootFile(eventsAndSolutions, config["outputFile"].get<std::string>());
    }

    void Application::runOnGpu() const
    {
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> events = loadEvents(config["inputFile"]);

#ifdef MULTIPLY_EVENTS
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> newEvents = std::make_unique<std::vector<std::shared_ptr<Event>>>();
        auto x = config["multiplyEvents"];
        for (unsigned int i = 0; i < config["multiplyEvents"]; i++)
        {
            for (auto event : *events)
            {
                newEvents->push_back(std::shared_ptr<Event>(new Event(*event)));
            }
        }

        events.swap(newEvents);
#endif

        ComputingManager computingManager(ComputingWorker::Platform::GPU, config["gpuEventBuffers"], config["gpuComputingWorkers"]);

        auto executionTimeStart = std::chrono::high_resolution_clock::now();

        for (std::shared_ptr<Event> &event : *events)
        {
            while (!computingManager.addEvent(event))
                computingManager.update();
        }
        computingManager.waitUntillAllTasksCompleted();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> eventsAndSolutions = computingManager.transferSolutions();

        auto executionTimeEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(executionTimeEnd - executionTimeStart).count();
        auto elapsedTime_sec = round(float(elapsedTime) / 1e3) / 1e3;
        INFO("Computing " << events->size() << " events took " << elapsedTime_sec << " seconds");

        saveSolutionsInRootFile(eventsAndSolutions, config["outputFile"].get<std::string>());
    }

    ComputingWorker::Platform Application::getPlatformFromString(const std::string &platformStr)
    {
        if (platformStr == "cpu_no_sycl")
            return ComputingWorker::Platform::CPU_NO_SYCL;
        if (platformStr == "cpu")
            return ComputingWorker::Platform::CPU;
        else if (platformStr == "gpu")
            return ComputingWorker::Platform::GPU;
        else if (platformStr == "FPGA")
            return ComputingWorker::Platform::FPGA;
        else if (platformStr == "FPGA_EMULATOR")
            return ComputingWorker::Platform::FPGA_EMULATOR;

        return ComputingWorker::Platform::BAD_PLATFORM;
    }

    std::unique_ptr<std::vector<std::shared_ptr<Event>>> Application::loadEvents(const std::string &path) const
    {
        if (config["inputFileType"] == "root_spacepoints")
            return loadEventsFromSpacepointsRootFile(path);
        else
        {
            // * Reading other file types are not implemented yet.
            return std::unique_ptr<std::vector<std::shared_ptr<Event>>>();
        }
    }

    std::function<bool(float, float, float)> Application::selector(const std::string &settingName, bool defaultDecision) const
    {
        // defaultDecisionine selector, default one selects all points
        if (config.contains(settingName))
        {
            DEBUG("Nontrivial selector " << config[settingName]);
            struct region
            {
                float rmin, rmax, zmin, zmax;
            };
            std::vector<region> exclusionRegions;
            for (auto conf : config[settingName])
            {
                exclusionRegions.emplace_back(region{conf[0], conf[1], conf[2], conf[3]});
                DEBUG("Will skip points from region: rmin: "
                      << exclusionRegions.back().rmin
                      << " rmax: " << exclusionRegions.back().rmax
                      << " zmin: " << exclusionRegions.back().zmin
                      << " zmax: " << exclusionRegions.back().zmax);
            }
            return [exclusionRegions](float x, float y, float z)
            {
                float r = std::hypot(x, y);
                // check if the point belongs to any region
                for (auto region : exclusionRegions)
                {
                    if (region.rmin < r && r < region.rmax && region.zmin < z && z < region.zmax)
                        return true;
                }
                return false;
            };
        }
        DEBUG("Selecting all points");
        return [defaultDecision](float x, float y, float z)
        { return defaultDecision; };
    }

    std::unique_ptr<std::vector<std::shared_ptr<Event>>> Application::loadEventsFromSpacepointsRootFile(const std::string &path) const
    {
        std::unique_ptr<TFile> file(TFile::Open(path.c_str()));
        if (file == nullptr)
        {
            throw std::runtime_error("Can't open input file: " + path);
        }
        INFO("... Opened: " << path);
        const std::string treeName = "spacepoints";
        std::unique_ptr<TTree> hitsTree(file->Get<TTree>(treeName.c_str()));
        if (hitsTree == nullptr)
        {
            throw std::runtime_error("Can't access tree in the ROOT file: " + treeName);
        }
        INFO("... Accessed input tree: " << treeName << " in " << path);

        uint32_t eventId;
        float x;
        float y;
        float z;
        uint8_t layer = 0;

        hitsTree->SetBranchAddress("event_id", &eventId);
        hitsTree->SetBranchAddress("x", &x);
        hitsTree->SetBranchAddress("y", &y);
        hitsTree->SetBranchAddress("z", &z);

        std::map<Event::EventId, std::unique_ptr<std::vector<Point>>> points;
        auto inExcludedRZRegions = selector("excludeRZRegions");

        for (int i = 0; hitsTree->LoadTree(i) >= 0; i++)
        {
            hitsTree->GetEntry(i);

            if (not config.contains("event") ||
                (config.contains("event") && eventId == config["event"]))
            { // to analyse only a single event add "event" property in config file
                points.try_emplace(eventId, std::make_unique<std::vector<Point>>());
                if (not inExcludedRZRegions(x, y, z))
                {
                    points[eventId]->push_back(Point{x, y, z, layer});
                    CDEBUG(DISPLAY_RPHI, std::hypot(x, y) << "," << std::atan2(y, x) << "," << z << ":RPhi");
                    // DEBUG("Accepted point, r: " << std::hypot(x,y) << ", z: " << z);
                } // else DEBUG("Rejected point, r: " << std::hypot(x,y) << ", z: " << z);
            }
        }
        // clean events from those having hits in undesired region
        auto excludeEventsWithHitsInRZ = selector("excludeEventsWithHitsInRZ");
        std::unique_ptr<std::vector<std::shared_ptr<Event>>> events = std::make_unique<std::vector<std::shared_ptr<Event>>>();
        for (auto &idAndPoints : points)
        {
            bool takeIt = true;
            for (auto data : *idAndPoints.second)
            {
                if (excludeEventsWithHitsInRZ(data.x, data.y, data.z))
                {
                    // DEBUG("Point in exclusion region, r:" <<  std::hypot(data.x, data.y) << " z:" << data.z << " will skip the event");
                    takeIt = false;
                    break;
                }
            }
            if (takeIt)
            {
                events->push_back(std::make_shared<Event>(idAndPoints.first, std::move(idAndPoints.second)));
                // DEBUG("Will use event");
                CDEBUG(DISPLAY_OK_EVENTS, idAndPoints.first << ":Events");
            }
            else
            {
                // DEBUG("Skipped event because it has hits in exclusion region");
            }
        }
        return events;
    }

    void Application::saveSolutionsInRootFile(const std::unique_ptr<std::vector<ComputingWorker::EventSoutionsPair>> &eventsAndSolutions, const std::string &path)
    {
        TFile *f = TFile::Open((path + ".root").c_str(), "RECREATE"); // we overwrite output, maybe this is wrong idea? TODO, decide
        uint32_t eventId{};
        float phi{};
        float pt{};
        float q{};
        float eta{};
        float z{};
        float d0{};
        int nhits{};

        TTree *outputTree = new TTree("solutions", "solutions");
        outputTree->Branch("event_id", &eventId);
        outputTree->Branch("phi", &phi);
        outputTree->Branch("pt", &pt);
        outputTree->Branch("eta", &eta);
        outputTree->Branch("q", &q);
        outputTree->Branch("z", &z);
        outputTree->Branch("d0", &d0);
        outputTree->Branch("nhits", &nhits);

        for (const auto &eventAndSolution : *eventsAndSolutions)
        {
            eventId = eventAndSolution.first->getId();

            for (const SolutionCircle &solution : *eventAndSolution.second)
            {
                if (solution.invalid())
                    break;

                phi = solution.phi;
                pt = solution.pt;
                q = solution.q;
                eta = solution.eta;
                z = solution.z;
                d0 = solution.d0;
                nhits = solution.nhits;

                outputTree->Fill();
            }
        }
        DEBUG("Output data saved to file " + path + ".root");
        f->Write();
        f->Close();
    }

    void Application::loadConfig(const std::string &configFilePath)
    {
        std::ifstream configFile(configFilePath);
        configFile >> config;
    }
} // HelixSolver