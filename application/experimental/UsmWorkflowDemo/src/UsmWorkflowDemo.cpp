#include "EventUsm/EventUsm.h"
#include "EventUsm/IWorkerController.h"
#include "EventUsm/QueueUsm.h"
#include "EventUsm/ResultUsm.h"
#include "EventUsm/TaskUsm.h"
#include "EventUsm/WorkerUsm.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"

#include <sycl/sycl.hpp>
#include <thread>


class DemoTask : public TaskUsm
{
public:
    explicit DemoTask(ITask::TaskId id)
    : TaskUsm(id) {}

    ~DemoTask() override = default;

    // From ITask
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override
    {
        ExecutionEvents executionEvents;

        std::unique_ptr<sycl::event> executionEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
            u_int32_t* numPoints = event_->kernelMemory_->numPoints_;
            float* xs = event_->kernelMemory_->xs_;
            float* ys = event_->kernelMemory_->ys_;
            float* zs = event_->kernelMemory_->zs_;
            EventUsm::LayerNumber* layers = event_->kernelMemory_->layers_;

            [[maybe_unused]] u_int32_t* numSolutions = result_->kernelMemory_->numSolutions_;
            [[maybe_unused]] u_int32_t* numRegionSolutions = result_->kernelMemory_->regionNumSolutions_;
            [[maybe_unused]] u_int8_t* solutionHitCounts = result_->kernelMemory_->solutionHitCounts_;
            float* solutionRs = result_->kernelMemory_->solutionRs_;
            float* solutionPhis = result_->kernelMemory_->solutionPhis_;
            
            handler.parallel_for(sycl::range<1>(event_->hostNumPoints_), [=](sycl::id<1> idx)
            {
                solutionRs[idx] = xs[idx] + ys[idx] + zs[idx] + layers[idx] + *numPoints;
                solutionPhis[idx] = xs[idx] + ys[idx] + zs[idx] + layers[idx] + *numPoints;
            });
        }));
        executionEvents.insert(std::move(executionEvent));

        return executionEvents;
    }
};

class DemoWorkerController : public IWorkerController
{
public:
    explicit DemoWorkerController() = default;

    ~DemoWorkerController() = default;

    // From IWorkerController
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

    // Create events
    std::vector<std::unique_ptr<EventUsm>> events;
    constexpr unsigned numEvents = 100;
    for(unsigned i = 0; i < numEvents; ++i)
    {
        std::unique_ptr<EventUsm> event = std::make_unique<EventUsm>(i);
        constexpr u_int32_t numPoints = 1000;
        event->hostNumPoints_ = numPoints;
        for(u_int32_t j = 0; j < numPoints; ++j)
        {
            event->hostXs_[j] = 1.0f;
            event->hostYs_[j] = 2.0f;
            event->hostZs_[j] = 3.0f;
            event->hostLayers_[j] = 4;
        }
        events.push_back(std::move(event));
    }

    // Create results
    std::vector<std::unique_ptr<ResultUsm>> results;
    constexpr unsigned numResults = 100;
    for(unsigned i = 0; i < numResults; ++i)
    {
        std::unique_ptr<ResultUsm> result = std::make_unique<ResultUsm>(i);
        results.push_back(std::move(result));
    }

    // Create tasks
    std::vector<std::unique_ptr<DemoTask>> tasks;
    constexpr unsigned numTasks = 100;
    for(unsigned i = 0; i < numTasks; ++i)
    {
        std::unique_ptr<DemoTask> task = std::make_unique<DemoTask>(i);
        task->takeEventAndResult(std::move(events[i]), std::move(results[i]));
        tasks.push_back(std::move(task));
    }

    // Create queue
    sycl::queue syclQueue = sycl::queue(sycl::gpu_selector_v);
    constexpr IQueue::Capacity ResourcesCapacity{10};
    constexpr IQueue::Capacity WorkCapacity{5};
    QueueUsm queueUsm(syclQueue, ResourcesCapacity, WorkCapacity);

    // Create kernel memory resources
    IQueue::CreateResourceGroupFunction createResources = [&](sycl::queue& syclQueue) -> std::unique_ptr<DeviceResourceGroup> {
        auto* eventMemory = new EventUsm::EventKernelMemory{syclQueue};
        auto* resultMemory = new ResultUsm::ResultKernelMemory{syclQueue};
        eventMemory->allocate();
        resultMemory->allocate();
        return std::make_unique<DeviceResourceGroup>(DeviceResourceGroup{
            {DeviceResourceType::EventKernelMemory, eventMemory},
            {DeviceResourceType::ResultKernelMemory, resultMemory}
        });
    };
    queueUsm.createResources(createResources);

    // Create worker controller
    DemoWorkerController workerController;

    // Create worker
    WorkerUsm workerUsm(queueUsm, workerController);

    // Submit tasks
    for(auto& task : tasks)
    {
        workerUsm.submitTask(std::move(task));
    }

    // Process tasks
    while (workerUsm.getNumberOfTasks() > 0)
    {
        workerUsm.processTasks();
        // Sleep for milisecond
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}