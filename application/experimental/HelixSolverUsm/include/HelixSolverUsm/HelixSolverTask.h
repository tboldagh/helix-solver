#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/TaskUsm.h"
#include "SplitterUsm/Splitter.h"
#include "HelixSolverUsm/SingleRegionKernelMemory.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest_prod.h>


class HelixSolverTask : public TaskUsm
{
public:
    explicit HelixSolverTask(ITask::TaskId id, const Splitter& splitter)
    : TaskUsm(id)
    , splitter_(splitter) {}

    ~HelixSolverTask() override = default;

    void takeResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resources) override;

    // From TaskUsm
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override;

protected:
    Splitter splitter_;
    SingleRegionKernelMemory* singleRegionKernelMemory_ = nullptr;

    FRIEND_TEST(HelixSolverTaskInitTest, TakeResources);
    friend class HelixSolverTaskExecutionTest;
    FRIEND_TEST(HelixSolverTaskFullEvent, Basic);
};