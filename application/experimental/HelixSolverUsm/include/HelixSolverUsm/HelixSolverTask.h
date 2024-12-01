#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/TaskUsm.h"
#include "SplitterUsm/Splitter.h"

#include <CL/sycl.hpp>
#include <gtest/gtest_prod.h>


class HelixSolverTask : public TaskUsm
{
public:
    explicit HelixSolverTask(ITask::TaskId id, const Splitter& splitter)
    : TaskUsm(id)
    , splitter_(splitter) {}

    ~HelixSolverTask() override = default;

    void takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources) override;

    // From TaskUsm
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override;

protected:
    bool splitterResourcesAssigned_ = false;
    Splitter* deviceSplitter_ = nullptr;
    // Must be same as deviceSplitter_, a bit ugly
    const Splitter splitter_;

    FRIEND_TEST(HelixSolverTaskTest, TakeEventResources);
};