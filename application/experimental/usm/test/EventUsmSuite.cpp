#include "EventUsm/EventUsm.h"

#include "gtest/gtest.h"
#include <CL/sycl.hpp>

class EventUsmAllocation : public ::testing::Test
{
protected:
    EventUsmAllocation()
    {
        queue_ = sycl::queue(sycl::gpu_selector());
    }

    sycl::queue queue_;
};

TEST_F(EventUsmAllocation, Allocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));

    ASSERT_TRUE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocation, NoDoubleAllocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));
    ASSERT_FALSE(event.allocateOnDevice(queue_));

    ASSERT_TRUE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocation, NoDoubleDeallocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));
 
    ASSERT_TRUE(event.deallocateOnDevice(queue_));
    ASSERT_FALSE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocation, MissingDeallocationLogged)
{
    EventUsm* event = new EventUsm(42);
    ASSERT_TRUE(event->allocateOnDevice(queue_));

    u_int32_t* deviceNumPoints = event->deviceNumPoints_;
    float* deviceXs = event->deviceXs_;
    float* deviceYs = event->deviceYs_;
    float* deviceZs = event->deviceZs_;
    EventUsm::LayerNumber* deviceLayers = event->deviceLayers_;

    delete event;
    // TODO: stub logger and check if error is logged

    sycl::free(deviceNumPoints, queue_);
    sycl::free(deviceXs, queue_);
    sycl::free(deviceYs, queue_);
    sycl::free(deviceZs, queue_);
    sycl::free(deviceLayers, queue_);
}

class EventUsmTransfer : public EventUsmAllocation
{
protected:
    EventUsmTransfer()
    : EventUsmAllocation()
    {
        event_ = EventUsm(42);
        event_.allocateOnDevice(queue_);
    }

    ~EventUsmTransfer()
    {
        event_.deallocateOnDevice(queue_);
    }

    EventUsm event_;
};

TEST_F(EventUsmTransfer, TransferToDevice)
{
    u_int32_t numPoints = 2137;
    event_.hostNumPoints_ = numPoints;
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        event_.hostXs_[i] = i;
        event_.hostYs_[i] = i;
        event_.hostZs_[i] = i;
        event_.hostLayers_[i] = i;
    }

    ASSERT_TRUE(event_.transferToDevice(queue_));

    // Double all 
    queue_.submit([&](sycl::handler& cgh)
    {
        cgh.parallel_for(sycl::range<1>(numPoints), [=](sycl::id<1> idx)
        {
            event_.deviceXs_[idx] *= 2;
            event_.deviceYs_[idx] *= 2;
            event_.deviceZs_[idx] *= 2;
            event_.deviceLayers_[idx] *= 2;
        });
    });
    queue_.wait();
}
