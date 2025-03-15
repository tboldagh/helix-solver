#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"


EventUsm::EventKernelMemory::EventKernelMemory(sycl::queue& queue)
: KernelMemory(queue) {}

void EventUsm::EventKernelMemory::allocateInternal()
{
    numPoints_ = sycl::malloc_device<u_int32_t>(1, queue_);
    xs_ = sycl::malloc_device<float>(EventUsm::MaxPoints, queue_);
    ys_ = sycl::malloc_device<float>(EventUsm::MaxPoints, queue_);
    zs_ = sycl::malloc_device<float>(EventUsm::MaxPoints, queue_);
    layers_ = sycl::malloc_device<EventUsm::LayerNumber>(EventUsm::MaxPoints, queue_);
}

void EventUsm::EventKernelMemory::deallocateInternal()
{
    sycl::free(numPoints_, queue_);
    sycl::free(xs_, queue_);
    sycl::free(ys_, queue_);
    sycl::free(zs_, queue_);
    sycl::free(layers_, queue_);
}


EventUsm::EventUsm(EventId eventId)
: eventId_(eventId)
, hostXs_(new float[MaxPoints])
, hostYs_(new float[MaxPoints])
, hostZs_(new float[MaxPoints])
, hostLayers_(new LayerNumber[MaxPoints]) {}

EventUsm::~EventUsm()
{
    delete[] hostXs_;
    delete[] hostYs_;
    delete[] hostZs_;
    delete[] hostLayers_;
}

TransferableData::TransferEvents EventUsm::transferToDevice()
{
    if (!isKernelMemorySet())
    {
        LOG_ERROR("Kernel memory not set for EventUsm, eventId: " + std::to_string(eventId_));
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        auto queue = kernelMemory_->getQueue();
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->numPoints_, &hostNumPoints_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->xs_, hostXs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->ys_, hostYs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->zs_, hostZs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(kernelMemory_->layers_, hostLayers_, hostNumPoints_ * sizeof(LayerNumber))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to device for EventUsm, eventId " + std::to_string(eventId_) + ", exception: " + exception.what());
        return TransferEvents{};
    }

    return transferEvents;
}

TransferableData::TransferEvents EventUsm::transferToHost()
{
    if (!isKernelMemorySet())
    {
        LOG_ERROR("Kernel memory not set for EventUsm, eventId: " + std::to_string(eventId_));
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        auto queue = kernelMemory_->getQueue();
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(&hostNumPoints_, kernelMemory_->numPoints_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostXs_, kernelMemory_->xs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostYs_, kernelMemory_->ys_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostZs_, kernelMemory_->zs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostLayers_, kernelMemory_->layers_, hostNumPoints_ * sizeof(LayerNumber))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to host for EventUsm, eventId " + std::to_string(eventId_) + ", exception: " + exception.what());
        return TransferEvents{};
    }

    return transferEvents;
}

void EventUsm::copyHostData(const EventUsm& source, EventUsm& destination)
{
    destination.hostNumPoints_ = source.hostNumPoints_;
    std::copy(source.hostXs_, source.hostXs_ + source.hostNumPoints_, destination.hostXs_);
    std::copy(source.hostYs_, source.hostYs_ + source.hostNumPoints_, destination.hostYs_);
    std::copy(source.hostZs_, source.hostZs_ + source.hostNumPoints_, destination.hostZs_);
    std::copy(source.hostLayers_, source.hostLayers_ + source.hostNumPoints_, destination.hostLayers_);
}

void EventUsm::setKernelMemoryInternal(KernelMemory* kernelMemory)
{
    kernelMemory_ = static_cast<EventKernelMemory*>(kernelMemory);
}
