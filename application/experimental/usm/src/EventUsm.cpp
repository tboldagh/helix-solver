#include "EventUsm/EventUsm.h"


EventUsm::EventUsm(EventId eventId)
: eventId_(eventId) {}

EventUsm::~EventUsm()
{
    if (allocated_)
    {
        // TODO: log error (memory leak)
    }
}

bool EventUsm::allocateOnDevice(sycl::queue& queue)
{
    if (allocated_)
    {
        // TODO: log error
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
        // TODO: log error
        return false;
    }

    allocated_ = true;
    return true;
}

bool EventUsm::transferToDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        // TODO: log error
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
        // TODO: log error
        return false;
    }
}

bool EventUsm::transferToHost(sycl::queue& queue)
{
    if (!allocated_)
    {
        // TODO: log error
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
        // TODO: log error
        return false;
    }

    return true;
}

bool EventUsm::deallocateOnDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        // TODO: log error
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
        // TODO: log error
        return false;
    }

    allocated_ = false;
    return true;
}

