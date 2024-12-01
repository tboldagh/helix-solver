#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"


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
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Run SplitterOnlyKernel for each region
        SingleRegionKernel kernel(deviceSplitter_, event_.get(), result_.get());
        handler.parallel_for(sycl::range<1>(numRegions + 1), kernel);   // numRegions + 1 because 0 is reserved for invalid region
    }));
    executionEvents.insert(std::move(executionEvent));
    return executionEvents;
}

