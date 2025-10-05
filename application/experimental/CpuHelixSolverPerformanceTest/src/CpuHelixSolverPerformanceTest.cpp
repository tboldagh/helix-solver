#include "CpuHelixSolver/HelixSolver.h"
#include "SplitterUsm/Splitter.h"
#include "CpuHelixSolver/Event.h"
#include "CpuHelixSolver/Result.h"
#include "CpuHelixSolver/Task.h"
#include "Logger/OstreamLogger.h"
#include "Logger/Logger.h"
#include "RootEventLoader/RootEventLoader.h"

int main()
{
    Logger::OstreamLogger logger(std::cout);
    logger.setMinSeverity(Logger::LogMessage::Severity::Debug);
    Logger::ILogger::setGlobalInstance(&logger);

    // Create splitter
    constexpr float maxAbsXy = 1100.0;
    constexpr float maxAbsZ = 3100.0;
    constexpr float minZAngle = 0.0;
    constexpr float maxZAngle = 2.0 * M_PI;
    constexpr float minXAngle = 1.0 / 16 * M_PI;
    constexpr float maxXAngle = 15.0 / 16 * M_PI;
    constexpr float poleRegionAngle = 1.0 / 16 * M_PI;
    constexpr float interactionRegionMin = -200.0;
    constexpr float interactionRegionMax = 200.0;
    constexpr float zAngleMargin = 4.0 / 256 * M_PI;
    constexpr float xAngleMargin = 2.0 / 256 * M_PI;
    constexpr u_int8_t numZRanges = 16;
    constexpr u_int8_t numXRanges = 32;
    constexpr float filterOutCenterR = 140.0;
    constexpr float filterOutCenterZ = 550.0;
    SplitterSettings splitterSettings(
        maxAbsXy, maxAbsZ,
        minZAngle, maxZAngle,
        minXAngle, maxXAngle,
        poleRegionAngle,
        interactionRegionMin, interactionRegionMax,
        zAngleMargin, xAngleMargin,
        numZRanges, numXRanges,
        filterOutCenterR, filterOutCenterZ
    );

    // Set wedges solution hits threshold and lines crossings threshold based on xAngles
    for (u_int16_t regionId = 1; regionId <= splitterSettings.wedges_.getSize(); ++regionId)
    {
        SplitterSettings::Wedge& region = splitterSettings.wedges_[regionId - 1];

        if (region.xAngleMin_ < 0.2f || region.xAngleMin_ > 2.5f)
        {
            region.solutionHitsThreshold_ = 3;
            region.linesCrossingsThreshold_ = 2;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 0.8f || region.xAngleMin_ > 2.0f)
        {
            region.solutionHitsThreshold_ = 3;
            region.linesCrossingsThreshold_ = 2;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 1.0f || region.xAngleMin_ > 1.8f)
        {
            region.solutionHitsThreshold_ = 3;
            region.linesCrossingsThreshold_ = 2;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 1.4f || region.xAngleMin_ > 1.6f)
        {
            region.solutionHitsThreshold_ = 3;
            region.linesCrossingsThreshold_ = 2;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else
        {
            region.solutionHitsThreshold_ = 3;
            region.linesCrossingsThreshold_ = 2;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
    }

    Splitter splitter(splitterSettings);

    HelixSolver helixSolver(splitter);

    {   // Lightweight events test
        LOG_INFO("Lightweight events test");
        const std::string dataPath = "/helix/repo/data/odd_output_singleMu_1000/spacepoints.root";
        RootEventLoader eventLoader;
        if (!eventLoader.setInputFile(dataPath))
        {
            LOG_ERROR("Failed to set input file to " + dataPath);
            return 1;
        }

        LOG_INFO("Creating tasks");
        std::vector<std::unique_ptr<Event>> events;
        std::vector<std::unique_ptr<Result>> results;
        std::vector<std::unique_ptr<Task>> tasks;
        const std::vector<u_int32_t> eventIds = eventLoader.getEventIds();
        events.reserve(eventIds.size());
        results.reserve(eventIds.size());
        tasks.reserve(eventIds.size());
        // for (u_int32_t i = 0; i < eventIds.size(); ++i)
        for (u_int32_t i = 0; i < 10; ++i)
        {
            const auto& eventId = eventIds[i];
            auto& event = *events.emplace_back(std::make_unique<Event>(eventId));
            auto& result = *results.emplace_back(std::make_unique<Result>(eventId));
            tasks.emplace_back(std::make_unique<Task>(event, result));

            eventLoader.loadEvent(eventId, event.xs_, event.ys_, event.zs_, &event.numPoints_);
        }
        LOG_INFO("Tasks created");

        LOG_INFO("Solving tasks");
        for (u_int32_t i = 0; i < tasks.size(); ++i)
        {
            auto& task = *tasks[i];

            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            helixSolver.solve(task);
            
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::stringstream ss;
            ss << "Task with event id " << task.getEvent().eventId_ << " solved in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms, num solutions: " << task.getResult().numSolutions_;
            LOG_INFO(ss.str());

            for (u_int32_t j = 0; j < task.getResult().numSolutions_; ++j)
            {
                const float q = task.getResult().solutionQs_[j];
                const float r = task.getResult().solutionRs_[j];
                const float phi = task.getResult().solutionPhis_[j];
                std::stringstream ss;
                ss << "Solution " << j << ": " << q << " " << r << " " << phi;
                LOG_INFO(ss.str());
            }
        }
        LOG_INFO("Tasks solved");

        LOG_INFO("Lightweight events test done");
    }   // Lightweight events test

    // {   // Heavy events test
    //     LOG_INFO("Heavy events test");
    //     const std::string dataPath = "/helix/repo/data/odd_output_ttbar_PU200_100/spacepoints.root";
    //     RootEventLoader eventLoader;
    //     if (!eventLoader.setInputFile(dataPath))
    //     {
    //         LOG_ERROR("Failed to set input file to " + dataPath);
    //         return 1;
    //     }

    //     LOG_INFO("Creating tasks");
    //     std::vector<std::unique_ptr<Event>> events;
    //     std::vector<std::unique_ptr<Result>> results;
    //     std::vector<std::unique_ptr<Task>> tasks;
    //     const std::vector<u_int32_t> eventIds = eventLoader.getEventIds();
    //     events.reserve(eventIds.size());
    //     results.reserve(eventIds.size());
    //     tasks.reserve(eventIds.size());
    //     // for (u_int32_t i = 0; i < eventIds.size(); ++i)
    //     for (u_int32_t i = 0; i < 10; ++i)
    //     {
    //         const auto& eventId = eventIds[i];
    //         auto& event = *events.emplace_back(std::make_unique<Event>(eventId));
    //         auto& result = *results.emplace_back(std::make_unique<Result>(eventId));
    //         tasks.emplace_back(std::make_unique<Task>(event, result));

    //         eventLoader.loadEvent(eventId, event.xs_, event.ys_, event.zs_, &event.numPoints_);
    //     }
    //     LOG_INFO("Tasks created");

    //     LOG_INFO("Solving tasks");
    //     std::vector<uint32_t> executionTimes;
    //     std::vector<uint32_t> numSolutions;
    //     for (u_int32_t i = 0; i < tasks.size(); ++i)
    //     {
    //         auto& task = *tasks[i];

    //         std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    //         helixSolver.solve(task);
            
    //         std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //         executionTimes.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    //         numSolutions.push_back(task.getResult().numSolutions_);
    //         std::stringstream ss;
    //         ss << "Task with event id " << task.getEvent().eventId_ << " solved in " << executionTimes[i] << " ms, num solutions: " << numSolutions[i];
    //         LOG_INFO(ss.str());
    //     }
    //     std::stringstream ss;
    //     ss << "Average execution time: " << std::accumulate(executionTimes.begin(), executionTimes.end(), 0) / executionTimes.size() << " ms, average number of solutions: " << std::accumulate(numSolutions.begin(), numSolutions.end(), 0) / numSolutions.size();
    //     LOG_INFO(ss.str());
    //     LOG_INFO("Tasks solved");

    //     LOG_INFO("Heavy events test done");
    // }   // Heavy events test
}