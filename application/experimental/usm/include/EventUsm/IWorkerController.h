#pragma once

#include <memory>

class ITask;

class IWorkerController
{
public:
    virtual ~IWorkerController() = default;

    virtual void onTaskCompleted(std::unique_ptr<ITask> task) = 0;
};