#include "EventUsm/WorkerUsm.h"
#include "Logger/Logger.h"

#include <string>


WorkerUsm::WorkerUsm(IQueue& queue, IWorkerController& workerController)
: queue_(queue)
, workerController_(workerController) {}

WorkerUsm::~WorkerUsm()
{
    // TODO
}

bool WorkerUsm::submitTask(std::unique_ptr<ITask> task)
{
    if(task.get() == nullptr)
    {
        LOG_ERROR("Attempt to submit nullptr");
        return false;
    }

    const ITask::State state = task->getState();
    const ITask::TaskId id = task->getId();

    if(state != ITask::State::EventAssigned)
    {
        LOG_ERROR("Attempt to submit task with invalid state, id: " + std::to_string(id) + " state: " + ITask::stateToString(state));
        return false;
    }

    if(tasks_.find(id) != tasks_.end())
    {
        LOG_ERROR("Attempt to submit task with id that already exists, id: " + std::to_string(id));
        return false;
    }

    tasks_.emplace(id, std::move(task));
    tasks_[id]->onAssignedToWorker();

    LOG_DEBUG("Task submitted, id: " + std::to_string(tasks_[id]->getId()));

    return true;
}

u_int16_t WorkerUsm::getNumberOfTasks() const
{
    return tasks_.size();
}

void WorkerUsm::processTasks()
{
    {   // Lock scope for tasksWithChangedState_
        std::lock_guard<std::mutex> lock(tasksWithChangedStateMutex_);
        for(ITask::TaskId id : tasksWithChangedState_)
        {
            tasksToProcess_.push_back(id);
        }
        tasksWithChangedState_.clear();
    }   // Lock scope for tasksWithChangedState_

    std::deque<ITask::TaskId> notProcessedTasks;

    while (!tasksToProcess_.empty())
    {
        ITask::TaskId id = tasksToProcess_.front();
        tasksToProcess_.pop_front();
        ITask& task = *tasks_[id].get();

        LOG_DEBUG("Processing task after state change, id: " + std::to_string(id) + " new state: " + ITask::stateToString(task.getState()));

        if(task.isStateChanging())
        {
            LOG_ERROR("Task state is changing, id: " + std::to_string(id) + " state: " + ITask::stateToString(task.getState()));
            continue;
        }

        bool processed = false;
        switch(task.getState())
        {
            case ITask::State::Created: // Fall trhough
            case ITask::State::EventAssigned:
                LOG_ERROR("Task has invalid state, id: " + std::to_string(id) + " state: " + ITask::stateToString(task.getState()));
                processed = true;
                break;
            case ITask::State::ReadyToQueue:
                processed = handleTaskReadyToQueue(task);
                break;
            case ITask::State::WaitingForResources:
                break;
            case ITask::State::WaitingForEventTransfer:
                break;
            case ITask::State::WaitingForExecution:
                break;
            case ITask::State::Executed:
                break;
            case ITask::State::WaitingForResultTransfer:
                break;
            case ITask::State::Completed:
                break;
        }
        if (!processed)
        {
            notProcessedTasks.push_back(id);
        }
    }

    tasksToProcess_.swap(notProcessedTasks);
}

void WorkerUsm::onTaskStateChange(ITask& task)
{
    std::lock_guard<std::mutex> lock(tasksWithChangedStateMutex_);
    tasksWithChangedState_.insert(task.getId());    
}

bool WorkerUsm::handleTaskReadyToQueue(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    if(queue_.getWorkLoad() == queue_.getWorkCapacity())
    {
        LOG_DEBUG("Queue is full");
        return false;
    }

    task.assignQueue(queue_);
    LOG_DEBUG("Task assigned to queue, task id: " + std::to_string(task.getId()));

    return false;
}

bool WorkerUsm::handleTaskWaitingForResources(ITask& task)
{
    // TODO
    return false;
}

bool WorkerUsm::handleTaskWaitingForEventTransfer(ITask& task)
{
    // TODO
    return false;
}

bool WorkerUsm::handleTaskWaitingForExecution(ITask& task)
{
    // TODO
    return false;
}

bool WorkerUsm::handleTaskExecuted(ITask& task)
{
    // TODO
    return false;
}

bool WorkerUsm::handleTaskWaitingForResultTransfer(ITask& task)
{
    // TODO
    return false;
}

bool WorkerUsm::handleTaskCompleted(ITask& task)
{
    // TODO
    return false;
}
