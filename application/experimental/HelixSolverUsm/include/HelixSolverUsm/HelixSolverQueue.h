#pragma once

#include "EventUsm/QueueUsm.h"
#include "SplitterUsm/Splitter.h"

class HelixSolverQueue : public QueueUsm
{
public:
    HelixSolverQueue(sycl::queue& syclQueue, Capacity eventResourcesCapacity, Capacity resultResourcesCapacity, Capacity workCapacity, const Splitter& splitter);
    ~HelixSolverQueue() override;

private:
    Splitter splitter_;
};