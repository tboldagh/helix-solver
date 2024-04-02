#pragma once

#include "EventUsm/IWorker.h"
#include "EventUsm/ITask.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <deque>
#include <unordered_set>
#include <mutex>

class IQueue;
class IWorkerController;

class WorkerUsm : public IWorker
{
public:
    WorkerUsm(IQueue& queue, IWorkerController& workerController);
    ~WorkerUsm() override;

    bool submitTask(std::unique_ptr<ITask> task) override;
    u_int16_t getNumberOfTasks() const override;
    void processTasks() override;
    void onTaskStateChange(ITask& task) override;

private:
    bool handleTaskReadyToQueue(ITask& task);
    bool handleTaskWaitingForResources(ITask& task);
    bool handleTaskWaitingForEventTransfer(ITask& task);
    bool handleTaskWaitingForExecution(ITask& task);
    bool handleTaskExecuted(ITask& task);
    bool handleTaskWaitingForResultTransfer(ITask& task);
    bool handleTaskCompleted(ITask& task);

    IQueue& queue_;
    IWorkerController& workerController_;
    std::unordered_map<ITask::TaskId, std::unique_ptr<ITask>> tasks_;
    std::deque<ITask::TaskId> tasksToProcess_;
    std::unordered_set<ITask::TaskId> tasksWithChangedState_;
    std::mutex tasksWithChangedStateMutex_;
};