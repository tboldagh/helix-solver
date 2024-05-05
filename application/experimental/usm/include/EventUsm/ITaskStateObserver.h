#pragma once


class ITask;


class ITaskStateObserver
{
public:
    virtual ~ITaskStateObserver() = default;

    // Has to be safe to call from multiple threads
    virtual void onTaskStateChange(ITask& task) = 0;
};