#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
#include "HelixSolverUsm/SumRegionNumSolutionsKernel.h"
#include "HelixSolverUsm/ConcentrateSolutionsKernel.h"


void HelixSolverTask::takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources)
{
    TaskUsm::takeEventResources(eventResources);

    deviceSplitter_ = static_cast<Splitter*>(eventResources.second.at(DeviceResourceType::Splitter));
    splitterResourcesAssigned_ = true;
}

ITask::ExecutionEvents HelixSolverTask::executeOnDevice(sycl::queue& syclQueue)
{
    ExecutionEvents executionEvents;

    std::unique_ptr<sycl::event> houghEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Run SplitterOnlyKernel for each region
        SingleRegionKernel kernel(deviceSplitter_, event_.get(), result_.get());
        handler.parallel_for(sycl::range<1>(numRegions), kernel);
    }));
    executionEvents.insert(std::move(houghEvent));

    std::unique_ptr<sycl::event> sumEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        // Sum number of solutions for each region
        SumRegionNumSolutionsKernel sumRegionNumSolutionsKernel(result_.get());
        handler.single_task(sumRegionNumSolutionsKernel);
    }));
    executionEvents.insert(std::move(sumEvent));

    std::unique_ptr<sycl::event> concentrateEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Concentrate solutions
        ConcentrateSolutionsKernel concentrateSolutionsKernel(result_.get());
        handler.parallel_for(sycl::range<1>(numRegions), concentrateSolutionsKernel);
    }));
    executionEvents.insert(std::move(concentrateEvent));

    return executionEvents;
}

