#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/TaskUsm.h"
#include "SplitterUsm/Splitter.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest_prod.h>


class HelixSolverTask : public TaskUsm
{
public:
    explicit HelixSolverTask(ITask::TaskId id, const Splitter& splitter)
    : TaskUsm(id)
    , splitter_(splitter) {}

    ~HelixSolverTask() override = default;

    void takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources) override;
    void transferEvent() override;

    // From TaskUsm
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override;

protected:
    bool splitterResourcesAssigned_ = false;
    Splitter* deviceSplitter_ = nullptr;
    // Must be same as deviceSplitter_, a bit ugly
    const Splitter splitter_;

private:
    void transferEventToDeviceThread();

    FRIEND_TEST(HelixSolverTaskInitTest, TakeEventResources);
    friend class HelixSolverTaskExecutionTest;
    FRIEND_TEST(HelixSolverTaskFullEvent, Basic);
};