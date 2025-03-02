#include "HelixSolverUsm/SingleRegionKernelMemory.h"
#include "EventUsm/EventUsm.h"

#include <sycl/sycl.hpp>


SingleRegionKernelMemory::SingleRegionKernelMemory(sycl::queue& queue)
: KernelMemory(queue) {}


void SingleRegionKernelMemory::allocateOnDevice()
{
    indexes_ = sycl::malloc_device<u_int32_t>(MaxPointsInRegion, queue_);
    xs_ = sycl::malloc_device<float>(MaxPointsInRegion, queue_);
    ys_ = sycl::malloc_device<float>(MaxPointsInRegion, queue_);
    zs_ = sycl::malloc_device<float>(MaxPointsInRegion, queue_);
    layers_ = sycl::malloc_device<EventUsm::LayerNumber>(MaxPointsInRegion, queue_);
    pointLists_ = sycl::malloc_device<u_int32_t>(MaxPointListsPointsNum, queue_);
    rs_ = sycl::malloc_device<float>(MaxPointsInRegion, queue_);
    phis_ = sycl::malloc_device<float>(MaxPointsInRegion, queue_);
}

void SingleRegionKernelMemory::deallocateOnDevice()
{
    sycl::free(indexes_, queue_);
    sycl::free(xs_, queue_);
    sycl::free(ys_, queue_);
    sycl::free(zs_, queue_);
    sycl::free(layers_, queue_);
    sycl::free(pointLists_, queue_);
    sycl::free(rs_, queue_);
    sycl::free(phis_, queue_);
}
