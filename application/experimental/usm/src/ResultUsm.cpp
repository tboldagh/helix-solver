#include "EventUsm/ResultUsm.h"
#include "Logger/Logger.h"


ResultUsm::ResultKernelMemory::ResultKernelMemory(sycl::queue& queue)
: KernelMemory(queue) {}

void ResultUsm::ResultKernelMemory::allocateInternal()
{
    numSolutions_ = sycl::malloc_device<u_int32_t>(1, queue_);
    regionNumSolutions_ = sycl::malloc_device<u_int32_t>(ResultUsm::MaxRegions, queue_);
    solutionHitCounts_ = sycl::malloc_device<u_int8_t>(ResultUsm::MaxSolutions, queue_);
    solutionRs_ = sycl::malloc_device<float>(ResultUsm::MaxSolutions, queue_);
    solutionPhis_ = sycl::malloc_device<float>(ResultUsm::MaxSolutions, queue_);
}

void ResultUsm::ResultKernelMemory::deallocateInternal()
{
    sycl::free(numSolutions_, queue_);
    sycl::free(regionNumSolutions_, queue_);
    sycl::free(solutionHitCounts_, queue_);
    sycl::free(solutionRs_, queue_);
    sycl::free(solutionPhis_, queue_);
}

ResultUsm::ResultUsm(ResultId resultId)
: resultId_(resultId)
, hostRegionNumSolutions_(new u_int32_t[MaxRegions])
, hostSolutionHitCounts_(new u_int8_t[MaxSolutions])
, hostSolutionRs_(new float[MaxSolutions])
, hostSolutionPhis_(new float[MaxSolutions]) {}

ResultUsm::~ResultUsm()
{
    delete[] hostRegionNumSolutions_;
    delete[] hostSolutionHitCounts_;
    delete[] hostSolutionRs_;
    delete[] hostSolutionPhis_;
}

TransferableData::TransferEvents ResultUsm::transferToDevice()
{
    if (!isKernelMemorySet())
    {
        LOG_ERROR("Kernel memory not set for ResultUsm, resultId: " + std::to_string(resultId_));
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        auto queue = kernelMemory_->getQueue();
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->numSolutions_, &hostNumSolutions_, sizeof(hostNumSolutions_))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->regionNumSolutions_, hostRegionNumSolutions_, MaxRegions * sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->solutionHitCounts_, hostSolutionHitCounts_, hostNumSolutions_ * sizeof(u_int8_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->solutionRs_, hostSolutionRs_, hostNumSolutions_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->solutionPhis_, hostSolutionPhis_, hostNumSolutions_ * sizeof(float))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to device for ResultUsm, resultId " + std::to_string(resultId_) + ", exception: " + exception.what());
        return TransferEvents{};
    }

    return transferEvents;
}

TransferableData::TransferEvents ResultUsm::transferToHost()
{
    if (!isKernelMemorySet())
    {
        LOG_ERROR("Kernel memory not set for ResultUsm, resultId: " + std::to_string(resultId_));
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        auto queue = kernelMemory_->getQueue();
        queue.memcpy(&hostNumSolutions_, kernelMemory_->numSolutions_, sizeof(hostNumSolutions_)).wait();
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostRegionNumSolutions_, kernelMemory_->regionNumSolutions_, MaxRegions * sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionHitCounts_, kernelMemory_->solutionHitCounts_, hostNumSolutions_ * sizeof(u_int8_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionRs_, kernelMemory_->solutionRs_, hostNumSolutions_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionPhis_, kernelMemory_->solutionPhis_, hostNumSolutions_ * sizeof(float))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to host for ResultUsm, resultId " + std::to_string(resultId_) + ", exception: " + exception.what());
        return TransferEvents{};
    }

    return transferEvents;
}

void ResultUsm::copyHostData(const ResultUsm& source, ResultUsm& destination)
{
    destination.hostNumSolutions_ = source.hostNumSolutions_;
    std::copy(source.hostRegionNumSolutions_, source.hostRegionNumSolutions_ + MaxRegions, destination.hostRegionNumSolutions_);
    std::copy(source.hostSolutionHitCounts_, source.hostSolutionHitCounts_ + MaxSolutions, destination.hostSolutionHitCounts_);
    std::copy(source.hostSolutionRs_, source.hostSolutionRs_ + MaxSolutions, destination.hostSolutionRs_);
    std::copy(source.hostSolutionPhis_, source.hostSolutionPhis_ + MaxSolutions, destination.hostSolutionPhis_);
}

void ResultUsm::setKernelMemoryInternal(KernelMemory* kernelMemory)
{
    kernelMemory_ = static_cast<ResultKernelMemory*>(kernelMemory);
}
