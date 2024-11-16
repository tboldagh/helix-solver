#include "HelixSolverUsm/HelixSolverTask.h"


void HelixSolverTask::takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources)
{
    TaskUsm::takeEventResources(eventResources);

    deviceSplitter_ = static_cast<Splitter*>(eventResources.second.at(DeviceResourceType::Splitter));
    splitterResourcesAssigned_ = true;
}

ITask::ExecutionEvents HelixSolverTask::executeOnDevice(sycl::queue& syclQueue)
{
    ExecutionEvents executionEvents;

    std::unique_ptr<sycl::event> executionEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        // Do actual work here
    }));
    executionEvents.insert(std::move(executionEvent));

    return executionEvents;
}

