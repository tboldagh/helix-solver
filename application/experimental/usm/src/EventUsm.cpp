#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"


EventUsm::EventUsm(EventId eventId)
: eventId_(eventId) {}

EventUsm::~EventUsm()
{
    if (allocated_)
    {
        LOG_ERROR("Memory leak in EventUsm with eventId " + std::to_string(eventId_) + ". Memory was not deallocated on device before destruction.");
    }
}

bool EventUsm::allocateOnDevice(sycl::queue& queue)
{
    if (allocated_)
    {
        LOG_ERROR("Memory already allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        deviceNumPoints_ = sycl::malloc_device<u_int32_t>(1, queue);
        deviceXs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceYs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceZs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceLayers_ = sycl::malloc_device<LayerNumber>(MaxPoints, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to allocate memory on device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = true;
    allocationQueue_ = &queue;
    return true;
}

bool EventUsm::deallocateOnDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        queue.wait();
        sycl::free(deviceNumPoints_, queue);
        sycl::free(deviceXs_, queue);
        sycl::free(deviceYs_, queue);
        sycl::free(deviceZs_, queue);
        sycl::free(deviceLayers_, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to deallocate memory on device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = false;
    allocationQueue_ = nullptr;
    return true;
}

bool EventUsm::transferToDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    if (allocationQueue_ != &queue)
    {
        LOG_ERROR("Memory allocated on different queue for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        queue.memcpy(deviceNumPoints_, &hostNumPoints_, sizeof(u_int32_t));
        queue.memcpy(deviceXs_, hostXs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(deviceYs_, hostYs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(deviceZs_, hostZs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(deviceLayers_, hostLayers_, hostNumPoints_ * sizeof(LayerNumber));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    return true;
}

bool EventUsm::transferToHost(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    if (allocationQueue_ != &queue)
    {
        LOG_ERROR("Memory allocated on different queue for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        queue.memcpy(&hostNumPoints_, deviceNumPoints_, sizeof(u_int32_t));
        queue.memcpy(hostXs_, deviceXs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(hostYs_, deviceYs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(hostZs_, deviceZs_, hostNumPoints_ * sizeof(float));
        queue.memcpy(hostLayers_, deviceLayers_, hostNumPoints_ * sizeof(LayerNumber));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to host for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    return true;
}
