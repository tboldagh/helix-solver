#include "EventUsm/ResultUsm.h"
#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"

#include <gtest/gtest.h>
#include <sycl/sycl.hpp>


class ResultKernelMemoryTest : public ::testing::Test
{
protected:
    ResultKernelMemoryTest()
    {
        queue_ = sycl::queue(sycl::gpu_selector_v);
    }

    sycl::queue queue_;
};

TEST_F(ResultKernelMemoryTest, Allocation)
{
    ResultUsm::ResultKernelMemory memory(queue_);

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
    ASSERT_NE(nullptr, memory.numSolutions_);
    ASSERT_NE(nullptr, memory.regionNumSolutions_);
    ASSERT_NE(nullptr, memory.solutionHitCounts_);
    ASSERT_NE(nullptr, memory.solutionRs_);
    ASSERT_NE(nullptr, memory.solutionPhis_);

    memory.deallocate();
    ASSERT_FALSE(memory.isAllocated());
}

TEST_F(ResultKernelMemoryTest, NoDoubleAllocation)
{
    ResultUsm::ResultKernelMemory memory(queue_);

    memory.allocate();
    auto* numSolutions = memory.numSolutions_;
    auto* numRegionSolutions = memory.regionNumSolutions_;
    auto* solutionHitCounts = memory.solutionHitCounts_;
    auto* solutionRs = memory.solutionRs_;
    auto* solutionPhis = memory.solutionPhis_;

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
    ASSERT_EQ(numSolutions, memory.numSolutions_);
    ASSERT_EQ(numRegionSolutions, memory.regionNumSolutions_);
    ASSERT_EQ(solutionHitCounts, memory.solutionHitCounts_);
    ASSERT_EQ(solutionRs, memory.solutionRs_);
    ASSERT_EQ(solutionPhis, memory.solutionPhis_);

    memory.deallocate();
    ASSERT_FALSE(memory.isAllocated());
}

TEST_F(ResultKernelMemoryTest, AllocationDeallocationAllocation)
{
    ResultUsm::ResultKernelMemory memory(queue_);

    memory.allocate();
    memory.deallocate();

    memory.allocate();
    ASSERT_TRUE(memory.isAllocated());
}

class ResultUsmTest : public ::testing::Test
{
protected:
    ResultUsmTest()
    {
        Logger::ILogger::setGlobalInstance(&logger_);
    }

    ~ResultUsmTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::ILoggerMock logger_;
};

TEST_F(ResultUsmTest, CopyHostData)
{
    constexpr u_int32_t numPoints = ResultUsm::MaxRegions;

    ResultUsm source(42);
    source.hostNumSolutions_ = numPoints;
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        source.hostRegionNumSolutions_[i] = i;
        source.hostSolutionHitCounts_[i] = i % 256;
        source.hostSolutionRs_[i] = i;
        source.hostSolutionPhis_[i] = i;
    }

    ResultUsm destination(43);
    ResultUsm::copyHostData(source, destination);
    ASSERT_EQ(destination.hostNumSolutions_, source.hostNumSolutions_);
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        ASSERT_EQ(destination.hostRegionNumSolutions_[i], source.hostRegionNumSolutions_[i]);
        ASSERT_EQ(destination.hostSolutionHitCounts_[i], source.hostSolutionHitCounts_[i]);
        ASSERT_EQ(destination.hostSolutionRs_[i], source.hostSolutionRs_[i]);
        ASSERT_EQ(destination.hostSolutionPhis_[i], source.hostSolutionPhis_[i]);
    }
}

TEST_F(ResultUsmTest, SetKernelMemory)
{
    ResultUsm result(42);
    sycl::queue queue = sycl::queue(sycl::gpu_selector_v);
    ResultUsm::ResultKernelMemory kernelMemory(queue);
    result.setKernelMemory(&kernelMemory);
    ASSERT_TRUE(result.isKernelMemorySet());
    ASSERT_EQ(result.kernelMemory_, &kernelMemory);
}

TEST_F(ResultUsmTest, SetAndUnsetKernelMemory)
{
    ResultUsm result(42);
    sycl::queue queue = sycl::queue(sycl::gpu_selector_v);
    ResultUsm::ResultKernelMemory kernelMemory(queue);
    result.setKernelMemory(&kernelMemory);
    ASSERT_TRUE(result.isKernelMemorySet());
    ASSERT_EQ(result.kernelMemory_, &kernelMemory);

    result.setKernelMemory(nullptr);
    ASSERT_FALSE(result.isKernelMemorySet());
    ASSERT_EQ(result.kernelMemory_, nullptr);
}

class ResultUsmTransferTest : public ::testing::Test
{
protected:
    ResultUsmTransferTest()
    : result_(42)
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        queue_ = sycl::queue(sycl::gpu_selector_v);

        kernelMemory_.allocate();
        result_.setKernelMemory(&kernelMemory_);
    }

    ~ResultUsmTransferTest() override
    {
        kernelMemory_.deallocate();

        Logger::ILogger::setGlobalInstance(nullptr);
    }

    sycl::queue queue_;
    ResultUsm result_;
    ResultUsm::ResultKernelMemory kernelMemory_{queue_};
    Logger::ILoggerMock logger_;
};

TEST_F(ResultUsmTransferTest, Transfer)
{
    result_.hostNumSolutions_ = ResultUsm::MaxRegions;
    for (u_int32_t i = 0; i < result_.hostNumSolutions_; ++i)
    {
        result_.hostRegionNumSolutions_[i] = i;
        result_.hostSolutionHitCounts_[i] = i;
        result_.hostSolutionRs_[i] = i;
        result_.hostSolutionPhis_[i] = i;
    }

    {   // Transfer to device
        TransferableData::TransferEvents transferEvents = result_.transferToDevice();
        ASSERT_FALSE(transferEvents.empty());
        for (auto& transferEvent : transferEvents)
        {
            transferEvent->wait();
        }
    }

    // Double all
    queue_.submit([&](sycl::handler& handler)
    {
        u_int32_t* numSolutions = result_.kernelMemory_->numSolutions_;
        u_int32_t* numRegionSolutions = result_.kernelMemory_->regionNumSolutions_;
        u_int8_t* solutionHitCounts = result_.kernelMemory_->solutionHitCounts_;
        float* solutionRs = result_.kernelMemory_->solutionRs_;
        float* solutionPhis = result_.kernelMemory_->solutionPhis_;

        handler.parallel_for(sycl::range<1>(result_.hostNumSolutions_), [=](sycl::id<1> idx)
        {
            *numSolutions = 100;
            numRegionSolutions[idx] *= 2;
            solutionHitCounts[idx] = numRegionSolutions[idx] % 256;
            solutionRs[idx] *= 2;
            solutionPhis[idx] *= 2;
        });
    }).wait();

    {   // Transfer to host
        TransferableData::TransferEvents transferEvents = result_.transferToHost();
        ASSERT_FALSE(transferEvents.empty());
        for (auto& transferEvent : transferEvents)
        {
            transferEvent->wait();
        }
    }

    ASSERT_EQ(result_.hostNumSolutions_, 100);
    for (u_int32_t i = 0; i < result_.hostNumSolutions_; ++i)
    {
        ASSERT_EQ(result_.hostSolutionHitCounts_[i], i * 2 % 256);
        ASSERT_EQ(result_.hostSolutionRs_[i], i * 2);
        ASSERT_EQ(result_.hostSolutionPhis_[i], i * 2);
    }

    for (u_int32_t i = 0; i < ResultUsm::MaxRegions; ++i)
    {
        ASSERT_EQ(result_.hostRegionNumSolutions_[i], i * 2);
    }

    // Assert no unnecessary transfers
    for (u_int32_t i = 100; i < ResultUsm::MaxRegions; ++i)
    {
        ASSERT_EQ(result_.hostSolutionHitCounts_[i], i % 256);
        ASSERT_EQ(result_.hostSolutionRs_[i], i);
        ASSERT_EQ(result_.hostSolutionPhis_[i], i);
    }
}
