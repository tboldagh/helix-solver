#pragma once

#include "EventUsm/ITask.h"
#include "EventUsm/IQueue.h"

#include <map>
#include <memory>

class IWorkerController;

class IWorker
{
public:
    virtual ~IWorker() = default;

    virtual bool submitTask(std::unique_ptr<ITask> task) = 0;
    virtual u_int16_t getNumberOfTasks() const = 0;
    virtual void processTasks() = 0;
    virtual void onTaskStateChange(ITask& task) = 0;
};