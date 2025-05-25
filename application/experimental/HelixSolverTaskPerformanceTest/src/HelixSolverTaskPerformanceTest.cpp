#include "EventUsm/IWorkerController.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "RootEventLoader/RootEventLoader.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
#include "HelixSolverUsm/HelixSolverTask.h"
#include "SplitterUsm/SplitterSettings.h"
#include "EventUsm/QueueUsm.h"
#include "EventUsm/WorkerUsm.h"

#include <sycl/sycl.hpp>
#include <thread>
#include <vector>
#include <memory>

class WorkerController : public IWorkerController
{
public:
    void onTaskCompleted(std::unique_ptr<ITask>&& task) override
    {
        const auto& result = task->releaseResult();
        LOG_INFO("Task " + std::to_string(task->getId()) + " completed, solutions: " + std::to_string(result->hostNumSolutions_) + ", execution time: " + std::to_string(task->getExecutionTime().count()) + " ms");
    }
};

void processTasks(WorkerUsm& workerUsm)
{
    while (workerUsm.getNumberOfTasks() > 0)
    {
        workerUsm.processTasks();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    // Setup logging
    Logger::OstreamLogger logger(std::cout);
    logger.setMinSeverity(Logger::LogMessage::Severity::Info);
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
    constexpr u_int8_t numXRanges = 8;
    constexpr float filterOutCenterR = 150.0;
    constexpr float filterOutCenterZ = 500.0;
    const SplitterSettings splitterSettings = SplitterSettings(
        maxAbsXy, maxAbsZ,
        minZAngle, maxZAngle,
        minXAngle, maxXAngle,
        poleRegionAngle,
        interactionRegionMin, interactionRegionMax,
        zAngleMargin, xAngleMargin,
        numZRanges, numXRanges,
        filterOutCenterR, filterOutCenterZ
    );
    Splitter splitter(splitterSettings);

    // Create queue
    sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
    constexpr IQueue::Capacity ResourcesCapacity{16};
    constexpr IQueue::Capacity WorkCapacity{8};
    QueueUsm queueUsm(syclQueue, ResourcesCapacity, WorkCapacity);
    queueUsm.createResources(SingleRegionKernel::createResourceGroup);
    queueUsm.forEachResourceGroup([&](IQueue::DeviceResourceGroupId id, DeviceResourceGroup& resourceGroup)
    {
        splitter.setKernelMemory(static_cast<KernelMemory*>(resourceGroup.at(DeviceResourceType::SplitterSettingsKernelMemory)));
        for (auto& transferEvent : splitter.transferToDevice())
        {
            transferEvent->wait();
        }
    });

    // Create worker controller
    WorkerController workerController;

    // Create worker
    WorkerUsm workerUsm(queueUsm, workerController);

    {   // Lightweight events test
        LOG_INFO("Lightweight events test");
        const std::string dataPath = "/helix/repo/data/odd_output_singleMu_1000/spacepoints.root";
        RootEventLoader eventLoader;
        if (!eventLoader.setInputFile(dataPath))
        {
            LOG_ERROR("Failed to set input file to " + dataPath);
            return 1;
        }

        std::vector<std::unique_ptr<HelixSolverTask>> tasks;
        const auto& events = eventLoader.loadAllEvents();
        for (auto& event : *events)
        {
            const auto& eventId = event.first;
            auto& eventUsm = event.second;
            std::unique_ptr<HelixSolverTask> task = std::make_unique<HelixSolverTask>(eventId, splitter);
            task->takeEventAndResult(std::move(eventUsm), std::make_unique<ResultUsm>(eventId));
            tasks.push_back(std::move(task));
        }

        // Submit tasks
        for (uint32_t i = 0; i < 10; ++i)
        // for (uint32_t i = 0; i < tasks.size(); ++i)
        {
            workerUsm.submitTask(std::move(tasks[i]));
        }
        processTasks(workerUsm);
        LOG_INFO("Lightweight events test done");
    }   // Lightweight events test

    {   // Heavy events test
        LOG_INFO("Heavy events test");
        const std::string dataPath = "/helix/repo/data/odd_output_ttbar_PU200_100/spacepoints.root";
        RootEventLoader eventLoader;
        if (!eventLoader.setInputFile(dataPath))
        {
            LOG_ERROR("Failed to set input file to " + dataPath);
            return 1;
        }

        std::vector<std::unique_ptr<HelixSolverTask>> tasks;
        const auto& events = eventLoader.loadAllEvents();
        for (auto& event : *events)
        {
            const auto& eventId = event.first;
            auto& eventUsm = event.second;
            std::unique_ptr<HelixSolverTask> task = std::make_unique<HelixSolverTask>(eventId, splitter);
            task->takeEventAndResult(std::move(eventUsm), std::make_unique<ResultUsm>(eventId));
            tasks.push_back(std::move(task));
        }

        // Submit tasks
        for (uint32_t i = 0; i < 10; ++i)
        // for (uint32_t i = 0; i < tasks.size(); ++i)
        {
            workerUsm.submitTask(std::move(tasks[i]));
        }
        processTasks(workerUsm);
        LOG_INFO("Heavy events test done");
    }   // Heavy events test
    return 0;
}