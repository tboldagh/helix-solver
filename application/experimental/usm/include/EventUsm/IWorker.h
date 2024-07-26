#pragma once

#include "EventUsm/ITaskStateObserver.h"

#include <map>
#include <memory>


class IQueue;
class ITask;
class IWorkerController;


class IWorker : public ITaskStateObserver
{
public:
    virtual ~IWorker() = default;

    virtual bool submitTask(std::unique_ptr<ITask> task) = 0;
    virtual u_int16_t getNumberOfTasks() const = 0;
    virtual void processTasks() = 0;
};