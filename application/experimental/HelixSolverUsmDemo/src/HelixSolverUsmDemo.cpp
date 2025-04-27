#include "EventUsm/EventUsm.h"
#include "EventUsm/IWorkerController.h"
#include "EventUsm/QueueUsm.h"
#include "EventUsm/ResultUsm.h"
#include "EventUsm/WorkerUsm.h"
#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "SplitterUsm/Splitter.h"
#include "SplitterUsm/TestDataLoader.h"

#include <chrono>
#include <cmath>
#include <thread>


class WorkerController : public IWorkerController
{
public:
    void onTaskCompleted(std::unique_ptr<ITask>&& task) override
    {
        LOG_INFO("Task " + std::to_string(task->getId()) + " completed.");
    }
};

int main()
{
    // Setup logging
    Logger::OstreamLogger logger(std::cout);
    Logger::ILogger::setGlobalInstance(&logger);

    // Load events and create results
    std::vector<std::unique_ptr<EventUsm>> events;
    std::vector<std::unique_ptr<ResultUsm>> results;
    const std::string eventPath = "/helix/repo/application/experimental/SplitterUsm/test-data/event_0.csv";
    auto eventOptional = TestDataLoader::readEvent(eventPath, 0);
    if (!eventOptional)
    {
        LOG_ERROR("Failed to load event from " + eventPath);
        return 1;
    }
    events.emplace_back(std::unique_ptr<EventUsm>(eventOptional->release()));
    results.emplace_back(std::make_unique<ResultUsm>(0));
    // TODO: Load more events

    // Create splitter
    constexpr float maxAbsXy = 1100.0;
    constexpr float maxAbsZ = 3100.0;
    constexpr float minZAngle = 0.0;
    constexpr float maxZAngle = 2.0 * M_PI;
    constexpr float minXAgle = 1.0 / 16 * M_PI;
    constexpr float maxXAgle = 15.0 / 16 * M_PI;
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
        minXAgle, maxXAgle,
        poleRegionAngle,
        interactionRegionMin, interactionRegionMax,
        zAngleMargin, xAngleMargin,
        numZRanges, numXRanges,
        filterOutCenterR, filterOutCenterZ
    );
    Splitter splitter(splitterSettings);

    // Create tasks
    std::vector<std::unique_ptr<HelixSolverTask>> tasks;
    for (unsigned i = 0; i < events.size(); ++i)
    {
        std::unique_ptr<HelixSolverTask> task = std::make_unique<HelixSolverTask>(i, splitter);
        task->takeEventAndResult(std::move(events[i]), std::move(results[i]));
        tasks.push_back(std::move(task));
    }

    // Create queue
    sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
    constexpr IQueue::Capacity ResourcesCapacity{10};
    constexpr IQueue::Capacity WorkCapacity{5};
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

    // Submit tasks
    for (auto& task : tasks)
    {
        workerUsm.submitTask(std::move(task));
    }

    // Process tasks
    while (workerUsm.getNumberOfTasks() > 0)
    {
        workerUsm.processTasks();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}