#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"

#include <gtest/gtest.h>
#include <sycl/sycl.hpp>


class EventKernelMemoryTest : public ::testing::Test
{
protected:
    EventKernelMemoryTest()
    {
        queue_ = sycl::queue(sycl::gpu_selector_v);
    }

    sycl::queue queue_;
};

TEST_F(EventKernelMemoryTest, Allocation)
{
    EventUsm::EventKernelMemory memory(queue_);

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
    ASSERT_NE(nullptr, memory.numPoints_);
    ASSERT_NE(nullptr, memory.xs_);
    ASSERT_NE(nullptr, memory.ys_);
    ASSERT_NE(nullptr, memory.zs_);
    ASSERT_NE(nullptr, memory.layers_);

    memory.deallocate();
    ASSERT_FALSE(memory.isAllocated());
}

TEST_F(EventKernelMemoryTest, NoDoubleAllocation)
{
    EventUsm::EventKernelMemory memory(queue_);

    memory.allocate();
    auto* numPoints = memory.numPoints_;
    auto* xs = memory.xs_;
    auto* ys = memory.ys_;
    auto* zs = memory.zs_;
    auto* layers = memory.layers_;

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
    ASSERT_EQ(numPoints, memory.numPoints_);
    ASSERT_EQ(xs, memory.xs_);
    ASSERT_EQ(ys, memory.ys_);
    ASSERT_EQ(zs, memory.zs_);
    ASSERT_EQ(layers, memory.layers_);

    memory.deallocate();
    ASSERT_FALSE(memory.isAllocated());
}

TEST_F(EventKernelMemoryTest, AllocationDeallocationAllocation)
{
    EventUsm::EventKernelMemory memory(queue_);

    memory.allocate();
    memory.deallocate();

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
}

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
        source.hostLayers_[i] = i % 256;
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

TEST_F(EventUsmTest, SetKernelMemory)
{
    EventUsm event(42);
    sycl::queue queue = sycl::queue(sycl::gpu_selector_v);
    EventUsm::EventKernelMemory kernelMemory(queue);
    event.setKernelMemory(&kernelMemory);
    ASSERT_TRUE(event.isKernelMemorySet());
    ASSERT_EQ(event.kernelMemory_, &kernelMemory);
}

TEST_F(EventUsmTest, SetAndUnsetKernelMemory)
{
    EventUsm event(42);
    sycl::queue queue = sycl::queue(sycl::gpu_selector_v);
    EventUsm::EventKernelMemory kernelMemory(queue);
    event.setKernelMemory(&kernelMemory);
    ASSERT_TRUE(event.isKernelMemorySet());
    ASSERT_EQ(event.kernelMemory_, &kernelMemory);

    event.setKernelMemory(nullptr);
    ASSERT_FALSE(event.isKernelMemorySet());
    ASSERT_EQ(event.kernelMemory_, nullptr);
}

class EventUsmTransferTest : public ::testing::Test
{
protected:
    EventUsmTransferTest()
    : event_(42)
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        queue_ = sycl::queue(sycl::gpu_selector_v);

        kernelMemory_.allocate();
        event_.setKernelMemory(&kernelMemory_);
    }

    ~EventUsmTransferTest() override
    {
        kernelMemory_.deallocate();

        Logger::ILogger::setGlobalInstance(nullptr);
    }

    sycl::queue queue_;
    EventUsm event_;
    EventUsm::EventKernelMemory kernelMemory_{queue_};
    Logger::ILoggerMock logger_;
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

    {   // Transfer to device
        TransferableData::TransferEvents transferEvents = event_.transferToDevice();
        ASSERT_FALSE(transferEvents.empty());
        for (auto& transferEvent : transferEvents)
        {
            transferEvent->wait();
        }
    }

    // Double all 
    queue_.submit([&](sycl::handler& handler)
    {
        u_int32_t* numPoints = event_.kernelMemory_->numPoints_;
        float* xs = event_.kernelMemory_->xs_;
        float* ys = event_.kernelMemory_->ys_;
        float* zs = event_.kernelMemory_->zs_;
        EventUsm::LayerNumber* layers = event_.kernelMemory_->layers_;

        handler.parallel_for(sycl::range<1>(event_.hostNumPoints_), [=](sycl::id<1> idx)
        {
            *numPoints = 420;
            xs[idx] *= 2;
            ys[idx] *= 2;
            zs[idx] *= 2;
            layers[idx] = layers[idx] * 2 % 256;
        });
    }).wait();

    {   // Transfer to host
        TransferableData::TransferEvents transferEvents = event_.transferToHost();
        ASSERT_FALSE(transferEvents.empty());
        for (auto& transferEvent : transferEvents)
        {
            transferEvent->wait();
        }
    }

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
