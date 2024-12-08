#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"

#include <gtest/gtest.h>
#include <CL/sycl.hpp>


class EventUsmTest : public ::testing::Test
{
protected:
    EventUsmTest()
    {
        Logger::ILogger::setGlobalInstance(&logger_);
    }

    ~EventUsmTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::ILoggerMock logger_;
};

TEST_F(EventUsmTest, CopyHostData)
{
    constexpr u_int32_t numPoints = 2137;

    EventUsm source(42);
    source.hostNumPoints_ = numPoints;
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        source.hostXs_[i] = i;
        source.hostYs_[i] = i;
        source.hostZs_[i] = i;
        source.hostLayers_[i] = i;
    }

    EventUsm destination(43);
    EventUsm::copyHostData(source, destination);
    ASSERT_EQ(destination.hostNumPoints_, source.hostNumPoints_);
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        ASSERT_EQ(destination.hostXs_[i], source.hostXs_[i]);
        ASSERT_EQ(destination.hostYs_[i], source.hostYs_[i]);
        ASSERT_EQ(destination.hostZs_[i], source.hostZs_[i]);
        ASSERT_EQ(destination.hostLayers_[i], source.hostLayers_[i]);
    }
}

class EventUsmAllocationTest : public ::testing::Test
{
protected:
    EventUsmAllocationTest()
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        queue_ = sycl::queue(sycl::gpu_selector_v);
    }

    ~EventUsmAllocationTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    sycl::queue queue_;
    Logger::ILoggerMock logger_;
};

TEST_F(EventUsmAllocationTest, Allocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));

    ASSERT_TRUE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocationTest, NoDoubleAllocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));
    ASSERT_FALSE(event.allocateOnDevice(queue_));

    ASSERT_TRUE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocationTest, NoDoubleDeallocation)
{
    EventUsm event(42);
    ASSERT_TRUE(event.allocateOnDevice(queue_));
 
    ASSERT_TRUE(event.deallocateOnDevice(queue_));
    ASSERT_FALSE(event.deallocateOnDevice(queue_));
}

TEST_F(EventUsmAllocationTest, MissingDeallocationLogged)
{
    EventUsm* event = new EventUsm(42);
    ASSERT_TRUE(event->allocateOnDevice(queue_));

    u_int32_t* deviceNumPoints = event->deviceNumPoints_;
    float* deviceXs = event->deviceXs_;
    float* deviceYs = event->deviceYs_;
    float* deviceZs = event->deviceZs_;
    EventUsm::LayerNumber* deviceLayers = event->deviceLayers_;

    EXPECT_CALL(logger_, log(testing::AllOf(
        testing::Property(&Logger::LogMessage::getSeverity, Logger::LogMessage::Severity::Error),
        testing::Property(&Logger::LogMessage::getMessage, "Memory leak in EventUsm with eventId " + std::to_string(event->eventId_) + ". Memory was not deallocated on device before destruction.")
    )));

    delete event;

    sycl::free(deviceNumPoints, queue_);
    sycl::free(deviceXs, queue_);
    sycl::free(deviceYs, queue_);
    sycl::free(deviceZs, queue_);
    sycl::free(deviceLayers, queue_);
}

class EventUsmTransferTest : public EventUsmAllocationTest
{
protected:
    EventUsmTransferTest()
        : EventUsmAllocationTest()
        , event_(42)
    {
        event_.allocateOnDevice(queue_);
    }

    ~EventUsmTransferTest() override
    {
        event_.deallocateOnDevice(queue_);
    }

    EventUsm event_;
};

TEST_F(EventUsmTransferTest, Transfer)
{
    event_.hostNumPoints_ = 2137;
    for (u_int32_t i = 0; i < event_.hostNumPoints_; ++i)
    {
        event_.hostXs_[i] = i;
        event_.hostYs_[i] = i;
        event_.hostZs_[i] = i;
        event_.hostLayers_[i] = i;
    }

    ASSERT_FALSE(event_.transferToDevice(queue_).empty());

    // Double all 
    queue_.submit([&](sycl::handler& cgh)
    {
        u_int32_t* numPoints = event_.deviceNumPoints_;
        float* xs = event_.deviceXs_;
        float* ys = event_.deviceYs_;
        float* zs = event_.deviceZs_;
        EventUsm::LayerNumber* layers = event_.deviceLayers_;

        cgh.parallel_for(sycl::range<1>(event_.hostNumPoints_), [=](sycl::id<1> idx)
        {
            *numPoints = 420;
            xs[idx] *= 2;
            ys[idx] *= 2;
            zs[idx] *= 2;
            layers[idx] = layers[idx] * 2 % 256;
        });
    });
    queue_.wait();

    ASSERT_FALSE(event_.transferToHost(queue_).empty());
    ASSERT_EQ(event_.hostNumPoints_, 420);
    for (u_int32_t i = 0; i < 420; ++i)
    {
        ASSERT_EQ(event_.hostXs_[i], i * 2);
        ASSERT_EQ(event_.hostYs_[i], i * 2);
        ASSERT_EQ(event_.hostZs_[i], i * 2);
        ASSERT_EQ(event_.hostLayers_[i], i * 2 % 256);
    }

    // Assert no unnecessary transfers
    for (u_int32_t i = 420; i < 2137; ++i)
    {
        ASSERT_EQ(event_.hostXs_[i], i);
        ASSERT_EQ(event_.hostYs_[i], i);
        ASSERT_EQ(event_.hostZs_[i], i);
        ASSERT_EQ(event_.hostLayers_[i], i % 256);
    }
}

TEST_F(EventUsmTransferTest, NoAllocationBeforeTransferLogged)
{
    EventUsm* event = new EventUsm(42);

    EXPECT_CALL(logger_, log(testing::AllOf(
        testing::Property(&Logger::LogMessage::getSeverity, Logger::LogMessage::Severity::Error),
        testing::Property(&Logger::LogMessage::getMessage, "Memory not allocated on device for EventUsm with eventId " + std::to_string(event->eventId_) + ".")
    )));

    ASSERT_TRUE(event->transferToDevice(queue_).empty());
}

TEST_F(EventUsmTransferTest, TransferToDeviceWrongQueueLogged)
{
    sycl::queue otherQueue(sycl::gpu_selector_v);

    EXPECT_CALL(logger_, log(testing::AllOf(
        testing::Property(&Logger::LogMessage::getSeverity, Logger::LogMessage::Severity::Error),
        testing::Property(&Logger::LogMessage::getMessage, "Memory allocated on different queue for EventUsm with eventId " + std::to_string(event_.eventId_) + ".")
    )));

    ASSERT_TRUE(event_.transferToDevice(otherQueue).empty());
}

TEST_F(EventUsmTransferTest, TransferToHostWrongQueueLogged)
{
    ASSERT_FALSE(event_.transferToDevice(queue_).empty());

    EXPECT_CALL(logger_, log(testing::AllOf(
        testing::Property(&Logger::LogMessage::getSeverity, Logger::LogMessage::Severity::Error),
        testing::Property(&Logger::LogMessage::getMessage, "Memory allocated on different queue for EventUsm with eventId " + std::to_string(event_.eventId_) + ".")
    )));

    sycl::queue otherQueue(sycl::gpu_selector_v);
    ASSERT_TRUE(event_.transferToHost(otherQueue).empty());
}