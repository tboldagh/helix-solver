#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
#include "HelixSolverUsm/SumRegionNumSolutionsKernel.h"
#include "HelixSolverUsm/ConcentrateSolutionsKernel.h"
#include "HelixSolverUsm/SingleRegionKernelMemory.h"
#include "Logger/Logger.h"

#include <thread>


void HelixSolverTask::takeResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resources)
{
    singleRegionKernelMemory_ = static_cast<SingleRegionKernelMemory*>(resources.second.at(DeviceResourceType::KernelMemory));
    splitter_.setKernelMemory(static_cast<KernelMemory*>(resources.second.at(DeviceResourceType::SplitterSettingsKernelMemory)));
    TaskUsm::takeResources(resources);
}

ITask::ExecutionEvents HelixSolverTask::executeOnDevice(sycl::queue& syclQueue)
{
    ExecutionEvents executionEvents;

    // Zero out regionNumSolutions_
    syclQueue.memset(result_.get()->kernelMemory_->regionNumSolutions_, 0, sizeof(u_int32_t) * ResultUsm::MaxRegions).wait();

    std::unique_ptr<sycl::event> houghEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Run SplitterOnlyKernel for each region
        SingleRegionKernel kernel(&splitter_, event_.get(), result_.get(), singleRegionKernelMemory_);
        handler.parallel_for(sycl::range<1>(numRegions), kernel);
    }));
    const auto* houghEventPtr = houghEvent.get();
    executionEvents.insert(std::move(houghEvent));

    std::unique_ptr<sycl::event> sumEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        handler.depends_on(*houghEventPtr);

        // Sum number of solutions for each region
        SumRegionNumSolutionsKernel sumRegionNumSolutionsKernel(result_.get());
        handler.single_task(sumRegionNumSolutionsKernel);
    }));
    const auto* sumEventPtr = sumEvent.get();
    executionEvents.insert(std::move(sumEvent));

    std::unique_ptr<sycl::event> concentrateEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        handler.depends_on(*sumEventPtr);
        
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Concentrate solutions
        ConcentrateSolutionsKernel concentrateSolutionsKernel(result_.get());
        handler.parallel_for(sycl::range<1>(numRegions), concentrateSolutionsKernel);
    }));
    executionEvents.insert(std::move(concentrateEvent));

    return executionEvents;
}
