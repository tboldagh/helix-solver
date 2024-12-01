#include "EventUsm/TaskUsm.h"
#include "EventUsm/ITaskStateObserver.h"
#include "Logger/Logger.h"

#include <thread>


TaskUsm::TaskUsm(ITask::TaskId id)
: id_(id) {}

TaskUsm::~TaskUsm()
{
    // TODO
}

void TaskUsm::takeEventAndResult(std::unique_ptr<EventUsm>&& event, std::unique_ptr<ResultUsm>&& result)
{
    if (event_ != nullptr)
    {
        LOG_WARNING("Task already has an event assiged but it's taking a new one, task id: " + std::to_string(id_) + ", event id: " + std::to_string(event_->eventId_) + ", new event id: " + std::to_string(event->eventId_));
    }

    event_ = std::move(event);

    if (result_ != nullptr)
    {
        LOG_WARNING("Task already has a result assiged but it's taking a new one, task id: " + std::to_string(id_) + ", result id: " + std::to_string(result_->resultId_) + ", new result id: " + std::to_string(result->resultId_));
    }

    result_ = std::move(result);

    setState(State::EventAndResultAssigned);
}

void TaskUsm::onAssignedToWorker(ITaskStateObserver& stateObserver)
{
    stateObserver_ = &stateObserver;

    setState(State::ReadyToQueue);
}

void TaskUsm::assignQueue(IQueue& queue)
{
    queue_ = &queue;

    setState(State::WaitingForResources);
}

void TaskUsm::takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources)
{
    eventResourceGroupId_ = eventResources.first;
    event_->takeResourceGroup(eventResources.second, queue_->getQueue());
    eventResourcesAssigned_ = true;

    checkResourcesAssigned();
}

void TaskUsm::takeResultResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resultResources)
{
    resultResourceGroupId_ = resultResources.first;
    result_->takeResourceGroup(resultResources.second, queue_->getQueue());
    resultResourcesAssigned_ = true;

    checkResourcesAssigned();
}

void TaskUsm::transferEvent()
{
    isStateChanging_ = true;
    std::thread transferThread(&TaskUsm::transferEventToDeviceThread, this);
    transferThread.detach();
}

void TaskUsm::execute()
{
    isStateChanging_ = true;
    std::thread executionThread(&TaskUsm::executeThread, this);
    executionThread.detach();
}

void TaskUsm::transferResult()
{
    isStateChanging_ = true;
    std::thread transferThread(&TaskUsm::transferResultFromDeviceThread, this);
    transferThread.detach();
}

IQueue::DeviceResourceGroupId TaskUsm::releaseEventResourceGroup()
{
    event_->releaseResourceGroup();
    eventResourcesAssigned_ = false;
    setState(State::WaitingForResultTransfer);
    return eventResourceGroupId_;
}

IQueue::DeviceResourceGroupId TaskUsm::releaseResultResourceGroup()
{
    result_->releaseResourceGroup();
    resultResourcesAssigned_ = false;
    setState(State::Completed);
    return resultResourceGroupId_;
}

void TaskUsm::setState(State state)
{
    state_ = state;

    if (stateObserver_ == nullptr)
    {
        return;
    }
    
    stateObserver_->onTaskStateChange(*this);
}

void TaskUsm::checkResourcesAssigned()
{
    if (eventResourcesAssigned_ && resultResourcesAssigned_)
    {
        setState(State::WaitingForEventTransfer);
    }
}

void TaskUsm::transferEventToDeviceThread()
{
    sycl::queue& syclQueue = queue_->checkoutQueue();
    DataUsm::TransferEvents transferEvents = event_->transferToDevice(syclQueue);
    queue_->checkinQueue();

    for (auto& transferEvent : transferEvents)
    {
        transferEvent->wait();
    }

    isStateChanging_ = false;
    setState(State::WaitingForExecution);
}

void TaskUsm::executeThread()
{
    sycl::queue& syclQueue = queue_->checkoutQueue();
    executionStart_ = std::chrono::steady_clock::now();
    ITask::ExecutionEvents executionEvents = executeOnDevice(syclQueue);
    queue_->checkinQueue();

    for (auto& executionEvent : executionEvents)
    {
        executionEvent->wait();
    }
    executionEnd_ = std::chrono::steady_clock::now();

    isStateChanging_ = false;
    setState(State::Executed);
}

void TaskUsm::transferResultFromDeviceThread()
{
    sycl::queue& syclQueue = queue_->checkoutQueue();
    DataUsm::TransferEvents transferResults = result_->transferToHost(syclQueue);
    queue_->checkinQueue();

    for (auto& transferResult : transferResults)
    {
        transferResult->wait();
    }

    isStateChanging_ = false;
    setState(State::ResultTransferred);
}