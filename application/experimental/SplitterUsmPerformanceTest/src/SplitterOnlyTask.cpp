#include "SplitterUsmPerformanceTest/SplitterOnlyKernel.h"
#include "SplitterUsmPerformanceTest/SplitterOnlyTask.h"

ITask::ExecutionEvents SplitterOnlyTask::executeOnDevice(sycl::queue& syclQueue)
{
    ExecutionEvents executionEvents;
    std::unique_ptr<sycl::event> executionEvent = std::make_unique<sycl::event>(syclQueue.submit([&](sycl::handler& handler) {
        const u_int16_t numRegions = splitter_.getNumRegions();

        // Run SplitterOnlyKernel for each region
        SplitterOnlyKernel kernel(deviceSplitter_, event_.get(), result_.get());
        handler.parallel_for(sycl::range<1>(numRegions + 1), kernel);   // numRegions + 1 because 0 is reserved for invalid region
    }));
    executionEvents.insert(std::move(executionEvent));
    return executionEvents;
}