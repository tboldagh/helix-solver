#include "SplitterUsmPerformanceTest/SplitterOnlyTask.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/IWorkerController.h"
#include "EventUsm/QueueUsm.h"
#include "EventUsm/ResultUsm.h"
#include "EventUsm/TaskUsm.h"
#include "EventUsm/WorkerUsm.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "SplitterUsm/TestDataLoader.h"
#include "HelixSolverUsm/HelixSolverQueue.h"

#include <CL/sycl.hpp>
#include <thread>

class WorkerController : public IWorkerController
{
public:
    explicit WorkerController() = default;

    ~WorkerController() = default;

    // From IWorkerController
    void onTaskCompleted(std::unique_ptr<ITask>&& task) override
    {
        LOG_INFO("Task " + std::to_string(task->getId()) + " completed, kernel execution time: " + std::to_string(task->getExecutionTime().count()) + " ms.");
    }
};


int main()
{
    // Setup logging
    Logger::OstreamLogger logger(std::cout);
    Logger::ILogger::setGlobalInstance(&logger);

    const std::string testDataDir = "/helix/repo/application/experimental/SplitterUsm/test-data";
    const std::string eventPath = testDataDir + "/event_0.csv";
    
    // Read event
    std::optional<std::unique_ptr<EventUsm>> eventOptional = TestDataLoader::readEvent(eventPath, 1);
    if (!eventOptional.has_value())
    {
        LOG_ERROR("Failed to read event from " + eventPath);
        return 1;
    }
    std::unique_ptr<EventUsm> event = std::move(eventOptional.value());

    // Create result
    std::unique_ptr<ResultUsm> result = std::make_unique<ResultUsm>(1);

    // Create splitter
    constexpr float maxAbsXy = 1100.0;
    constexpr float maxAbsZ = 3100.0;
    constexpr float minZAngle = 0.0;
    constexpr float maxZAngle = 2.0 * M_PI;
    constexpr float minXAgle = 1.0 / 16 * M_PI;
    constexpr float maxXAgle = 15.0 / 16 * M_PI;
    constexpr float poleRegionAngle = 1.0 / 16 * M_PI;
    constexpr float interactionRegionMin = -250.0;
    constexpr float interactionRegionMax = 250.0;
    constexpr float zAngleMargin = 4.0 / 256 * M_PI;
    constexpr float xAngleMargin = 2.0 / 256 * M_PI;
    constexpr u_int8_t numZRanges = 16;
    constexpr u_int8_t numXRanges = 8;
    SplitterSettings splitterSettings(
        maxAbsXy, maxAbsZ,
        minZAngle, maxZAngle,
        minXAgle, maxXAgle,
        poleRegionAngle,
        interactionRegionMin, interactionRegionMax,
        zAngleMargin, xAngleMargin,
        numZRanges, numXRanges
    );
    const Splitter splitter(splitterSettings);

    // Create task
    auto task = std::make_unique<SplitterOnlyTask>(1, splitter);
    task->takeEventAndResult(std::move(event), std::move(result));

    // Create queue
    sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
    LOG_INFO("Running on device: " + syclQueue.get_device().get_info<sycl::info::device::name>());
    constexpr IQueue::Capacity EventResourcesCapacity{10};
    constexpr IQueue::Capacity ResultResourcesCapacity{10};
    constexpr IQueue::Capacity WorkCapacity{5};
    HelixSolverQueue queueUsm(syclQueue, EventResourcesCapacity, ResultResourcesCapacity, WorkCapacity, splitter);

    // Create worker controller
    WorkerController workerController;

    // Create worker
    WorkerUsm workerUsm(queueUsm, workerController);

    // Submit task
    workerUsm.submitTask(std::move(task));

    // Process tasks
    while (workerUsm.getNumberOfTasks() > 0)
    {
        workerUsm.processTasks();
        // Sleep for milisecond
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}