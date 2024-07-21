#include "EventUsm/WorkerUsm.h"
#include "EventUsm/IWorkerController.h"
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

    if(state != ITask::State::EventAndResultAssigned)
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
    tasks_[id]->onAssignedToWorker(*this);

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
            case ITask::State::EventAndResultAssigned:
                LOG_ERROR("Task has invalid state, id: " + std::to_string(id) + " state: " + ITask::stateToString(task.getState()));
                processed = true;
                break;
            case ITask::State::ReadyToQueue:
                processed = handleTaskReadyToQueue(task);
                break;
            case ITask::State::WaitingForResources:
                processed = handleTaskWaitingForResources(task);
                break;
            case ITask::State::WaitingForEventTransfer:
                processed = handleTaskWaitingForEventTransfer(task);
                break;
            case ITask::State::WaitingForExecution:
                processed = handleTaskWaitingForExecution(task);
                break;
            case ITask::State::Executed:
                processed = handleTaskExecuted(task);
                break;
            case ITask::State::WaitingForResultTransfer:
                processed = handleTaskWaitingForResultTransfer(task);
                break;
            case ITask::State::ResultTransferred:
                processed = handleResultTransferred(task);
                break;
            case ITask::State::Completed:
                processed = handleTaskCompleted(task);
                break;
            default:
                LOG_ERROR("Task has invalid state, id: " + std::to_string(id) + " state: " + ITask::stateToString(task.getState()));
                break;
        }
        if (!processed)
        {
            notProcessedTasks.push_back(id);
        }
    }

    tasksToProcess_.swap(notProcessedTasks);
    notProcessedTasks.clear();
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

    queue_.incrementWorkLoad();
    task.assignQueue(queue_);
    LOG_DEBUG("Task assigned to queue, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleTaskWaitingForResources(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));
    
    bool processed = true;

    if (!task.isEventResourcesAssigned())
    {
        LOG_DEBUG("Waiting for event resources");

        if (queue_.getEventResourcesLoad() == queue_.getEventResourcesCapacity())
        {
            LOG_DEBUG("Queue has no free event resources");
            processed = false;
        }
        else
        {
            task.takeEventResources(queue_.getEventResourceGroup());
            LOG_DEBUG("Event resources assigned, task id: " + std::to_string(task.getId()));
        }
    }

    if (!task.isResultResourcesAssigned())
    {
        LOG_DEBUG("Waiting for result resources");

        if (queue_.getResultResourcesLoad() == queue_.getResultResourcesCapacity())
        {
            LOG_DEBUG("Queue has no free result resources");
            processed = false;
        }
        else
        {
            task.takeResultResources(queue_.getResultResourceGroup());
            LOG_DEBUG("Result resources assigned, task id: " + std::to_string(task.getId()));
        }
    }

    return processed;
}

bool WorkerUsm::handleTaskWaitingForEventTransfer(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    task.transferEvent();
    LOG_DEBUG("Event transfer started, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleTaskWaitingForExecution(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    task.execute();
    LOG_DEBUG("Task execution started, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleTaskExecuted(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    queue_.returnEventResourceGroup(task.releaseEventResourceGroup());
    LOG_DEBUG("Event resources returned to queue, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleTaskWaitingForResultTransfer(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    task.transferResult();
    LOG_DEBUG("Result transfer started, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleResultTransferred(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    queue_.returnResultResourceGroup(task.releaseResultResourceGroup());
    LOG_DEBUG("Result resources returned to queue, task id: " + std::to_string(task.getId()));

    return true;
}

bool WorkerUsm::handleTaskCompleted(ITask& task)
{
    LOG_DEBUG("Task id: " + std::to_string(task.getId()));

    LOG_DEBUG("Task completed and will be handed in to controller, task id: " + std::to_string(task.getId()));
    queue_.decrementWorkLoad();
    workerController_.onTaskCompleted(std::unique_ptr<ITask>(tasks_.extract(task.getId()).mapped().release()));

    return true;
}
