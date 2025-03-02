#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
#include "HelixSolverUsm/SumRegionNumSolutionsKernel.h"
#include "HelixSolverUsm/ConcentrateSolutionsKernel.h"
#include "HelixSolverUsm/SingleRegionKernelMemory.h"
#include "Logger/Logger.h"

#include <thread>


void HelixSolverTask::takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources)
{
    TaskUsm::takeEventResources(eventResources);

    deviceSplitter_ = static_cast<Splitter*>(eventResources.second.at(DeviceResourceType::Splitter));
    splitterResourcesAssigned_ = true;
}

void HelixSolverTask::transferEvent()
{
    isStateChanging_ = true;
    std::thread transferThread(&HelixSolverTask::transferEventToDeviceThread, this);
    transferThread.detach();
}

ITask::ExecutionEvents HelixSolverTask::executeOnDevice(sycl::queue& syclQueue)
{
    SingleRegionKernelMemory kernelMemory(syclQueue);
    kernelMemory.allocateOnDevice();

    ExecutionEvents executionEvents;

    std::unique_ptr<sycl::event> houghEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Run SplitterOnlyKernel for each region
        SingleRegionKernel kernel(deviceSplitter_, event_.get(), result_.get(), kernelMemory);
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

    kernelMemory.deallocateOnDevice();

    return executionEvents;
}

void HelixSolverTask::transferEventToDeviceThread()
{
    sycl::queue& syclQueue = queue_->checkoutQueue();
    DataUsm::TransferEvents transferEvents = event_->transferToDevice(syclQueue);
    transferEvents.insert(std::make_unique<sycl::event>(syclQueue.memcpy(deviceSplitter_, &splitter_, sizeof(Splitter))));
    queue_->checkinQueue();

    for (auto& transferEvent : transferEvents)
    {
        transferEvent->wait();
    }

    isStateChanging_ = false;
    setState(State::WaitingForExecution);
}