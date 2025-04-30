#pragma once

#include "CpuHelixSolver/Event.h"
#include "CpuHelixSolver/Result.h"

class Task
{
public:
    Task(Event& event, Result& result)
    : event_(event)
    , result_(result) {}

    Event& getEvent() const { return event_; }
    Result& getResult() const { return result_; }

private:
    Event& event_;
    Result& result_;
};