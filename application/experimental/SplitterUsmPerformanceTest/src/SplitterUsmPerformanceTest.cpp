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
#include <numeric>

class SingleTaskTestWorkerController : public IWorkerController
{
public:
    explicit SingleTaskTestWorkerController() = default;

    ~SingleTaskTestWorkerController() = default;

    // From IWorkerController
    void onTaskCompleted(std::unique_ptr<ITask>&& task) override
    {
        LOG_INFO("Task " + std::to_string(task->getId()) + " completed, kernel execution time: " + std::to_string(task->getExecutionTime().count()) + " ms.");
    }
};

class MultipleTasksTestWorkerController : public IWorkerController
{
public:
    explicit MultipleTasksTestWorkerController() = default;

    ~MultipleTasksTestWorkerController() = default;

    // From IWorkerController
    void onTaskCompleted(std::unique_ptr<ITask>&& task) override
    {
        LOG_DEBUG("Task " + std::to_string(task->getId()) + " completed, kernel execution time: " + std::to_string(task->getExecutionTime().count()) + " ms.");
        
        executionTimes_.push_back(task->getExecutionTime().count());
    }

    const std::vector<std::chrono::milliseconds::rep>& getExecutionTimes() const
    {
        return executionTimes_;
    }

private:
    std::vector<std::chrono::milliseconds::rep> executionTimes_;
};

SplitterSettings getSplitterSettings()
{
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
    return SplitterSettings(
        maxAbsXy, maxAbsZ,
        minZAngle, maxZAngle,
        minXAgle, maxXAgle,
        poleRegionAngle,
        interactionRegionMin, interactionRegionMax,
        zAngleMargin, xAngleMargin,
        numZRanges, numXRanges
    );
}


int main()
{
    Logger::OstreamLogger logger(std::cout);
    logger.setMinSeverity(Logger::LogMessage::Severity::Info);
    Logger::ILogger::setGlobalInstance(&logger);

    const std::string testDataDir = "/helix/repo/application/experimental/SplitterUsm/test-data";
    const std::string eventPath = testDataDir + "/event_0.csv";

    const Splitter splitter(getSplitterSettings());

    std::optional<std::unique_ptr<EventUsm>> eventOptional = TestDataLoader::readEvent(eventPath, 1);
    if (!eventOptional.has_value())
    {
        LOG_ERROR("Failed to read event from " + eventPath);
        return 1;
    }
    std::unique_ptr<EventUsm> event = std::move(eventOptional.value());


    {   // Single task test scope
        LOG_INFO("#################### Single task test ####################");

        std::unique_ptr<EventUsm> eventCopy = std::make_unique<EventUsm>(1);
        EventUsm::copyHostData(*event, *eventCopy);

        std::unique_ptr<ResultUsm> result = std::make_unique<ResultUsm>(1);

        auto task = std::make_unique<SplitterOnlyTask>(1, splitter);
        task->takeEventAndResult(std::move(eventCopy), std::move(result));
        
        sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
        LOG_INFO("Running on device: " + syclQueue.get_device().get_info<sycl::info::device::name>());
        constexpr IQueue::Capacity EventResourcesCapacity{16};
        constexpr IQueue::Capacity ResultResourcesCapacity{16};
        constexpr IQueue::Capacity WorkCapacity{8};
        HelixSolverQueue queueUsm(syclQueue, EventResourcesCapacity, ResultResourcesCapacity, WorkCapacity, splitter);

        SingleTaskTestWorkerController workerController;

        WorkerUsm workerUsm(queueUsm, workerController);
    
        workerUsm.submitTask(std::move(task));

        while (workerUsm.getNumberOfTasks() > 0)
        {
            workerUsm.processTasks();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }   // Single task test scope


    // Same flow as in single task test but with multiple events, results and tasks
    {   // Multiple tasks test scope
        LOG_INFO("#################### Multiple tasks test ####################");

        constexpr u_int16_t numEvents = 1000;

        std::vector<std::unique_ptr<EventUsm>> events;
        for (u_int16_t i = 1; i <= numEvents; ++i)
        {
            events.emplace_back(std::make_unique<EventUsm>(i));
            EventUsm::copyHostData(*event, *events.back());
        }

        std::vector<std::unique_ptr<ResultUsm>> results;
        for (u_int16_t i = 1; i <= numEvents; ++i)
        {
            results.emplace_back(std::make_unique<ResultUsm>(i));
        }

        std::vector<std::unique_ptr<SplitterOnlyTask>> tasks;
        for (u_int16_t i = 1; i <= numEvents; ++i)
        {
            tasks.emplace_back(std::make_unique<SplitterOnlyTask>(i, splitter));
        }
        for (u_int16_t i = 0; i < numEvents; ++i)
        {
            tasks[i]->takeEventAndResult(std::move(events[i]), std::move(results[i]));
        }
        
        sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
        LOG_INFO("Running on device: " + syclQueue.get_device().get_info<sycl::info::device::name>());
        constexpr IQueue::Capacity EventResourcesCapacity{16};
        constexpr IQueue::Capacity ResultResourcesCapacity{16};
        constexpr IQueue::Capacity WorkCapacity{8};
        HelixSolverQueue queueUsm(syclQueue, EventResourcesCapacity, ResultResourcesCapacity, WorkCapacity, splitter);

        MultipleTasksTestWorkerController workerController;

        WorkerUsm workerUsm(queueUsm, workerController);
    
        for (auto& task : tasks)
        {
            workerUsm.submitTask(std::move(task));
        }

        while (workerUsm.getNumberOfTasks() > 0)
        {
            workerUsm.processTasks();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        const auto& executionTimes = workerController.getExecutionTimes();
        for (u_int16_t i = 0; i < 10; ++i)
        {
            LOG_INFO(std::to_string(i + 1) + "-th completed task execution time: " + std::to_string(executionTimes[i]) + " ms.");
        }
        LOG_INFO("Average first 10 tasks execution time: " + std::to_string(std::accumulate(executionTimes.begin(), executionTimes.begin() + 10, 0) / 10) + " ms.");
        LOG_INFO("Average first 100 tasks execution time: " + std::to_string(std::accumulate(executionTimes.begin(), executionTimes.begin() + 100, 0) / 100) + " ms.");
        LOG_INFO("Average [200, 799] tasks execution time: " + std::to_string(std::accumulate(executionTimes.begin() + 199, executionTimes.begin() + 800, 0) / 600) + " ms.");
        LOG_INFO("Average all tasks execution time: " + std::to_string(std::accumulate(executionTimes.begin(), executionTimes.begin() + numEvents, 0) / numEvents) + " ms.");
    }   // Multiple tasks test scope
}