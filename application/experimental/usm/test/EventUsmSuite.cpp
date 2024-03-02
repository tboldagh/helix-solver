#include "EventUsm/EventUsm.h"

#include <gtest/gtest.h>
#include <CL/sycl.hpp>

class EventUsmAllocation : public ::testing::Test
{
protected:
    EventUsmAllocation()
    {
        queue_ = sycl::queue(sycl::gpu_selector_v);
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
        , event_(42)
    {
        event_.allocateOnDevice(queue_);
    }

    ~EventUsmTransfer()
    {
        event_.deallocateOnDevice(queue_);
    }

    EventUsm event_;
};

TEST_F(EventUsmTransfer, Transfer)
{
    event_.hostNumPoints_ = 2137;
    for (u_int32_t i = 0; i < event_.hostNumPoints_; ++i)
    {
        event_.hostXs_[i] = i;
        event_.hostYs_[i] = i;
        event_.hostZs_[i] = i;
        event_.hostLayers_[i] = i;
    }

    ASSERT_TRUE(event_.transferToDevice(queue_));

    // Double all 
    queue_.submit([&, this](sycl::handler& cgh)
    {
        u_int32_t* numPoints = event_.deviceNumPoints_;
        float* xs = event_.deviceXs_;
        float* ys = event_.deviceYs_;
        float* zs = event_.deviceZs_;
        EventUsm::LayerNumber* layers = event_.deviceLayers_;

        cgh.parallel_for(sycl::range<1>(*numPoints), [=](sycl::id<1> idx)
        {
            numPoints[idx] = 420;
            xs[idx] *= 2;
            ys[idx] *= 2;
            zs[idx] *= 2;
            layers[idx] *= 2;
        });
    });
    queue_.wait();

    ASSERT_TRUE(event_.transferToHost(queue_));
    ASSERT_EQ(event_.hostNumPoints_, 420);
    for (u_int32_t i = 0; i < 420; ++i)
    {
        ASSERT_EQ(event_.hostXs_[i], i * 2);
        ASSERT_EQ(event_.hostYs_[i], i * 2);
        ASSERT_EQ(event_.hostZs_[i], i * 2);
        ASSERT_EQ(event_.hostLayers_[i], i * 2);
    }

    // Assert no unnecessary transfers
    for (u_int32_t i = 420; i < 2137; ++i)
    {
        ASSERT_EQ(event_.hostXs_[i], i);
        ASSERT_EQ(event_.hostYs_[i], i);
        ASSERT_EQ(event_.hostZs_[i], i);
        ASSERT_EQ(event_.hostLayers_[i], i);
    }
}
