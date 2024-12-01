#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/TaskUsm.h"
#include "SplitterUsm/Splitter.h"

#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"

#include <CL/sycl.hpp>
#include <gtest/gtest_prod.h>
#include <chrono>


class SplitterOnlyTask : public HelixSolverTask
{
public:
    explicit SplitterOnlyTask(ITask::TaskId id, const Splitter& splitter)
    : HelixSolverTask(id, splitter) {}

    ~SplitterOnlyTask() override = default;

    // From TaskUsm
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override;
};