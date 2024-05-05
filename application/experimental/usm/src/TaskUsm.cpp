#include "EventUsm/TaskUsm.h"

#include <thread>


TaskUsm::TaskUsm(ITask::TaskId id)
: id_(id) {}

TaskUsm::~TaskUsm()
{
    // TODO
}

void TaskUsm::takeEvent(std::unique_ptr<EventUsm>&& event)
{
    event_ = std::move(event);

    setState(State::EventAssigned);
}

void TaskUsm::onAssignedToWorker(IStateObserver& stateObserver)
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
}

void TaskUsm::execute()
{
    isStateChanging_ = true;
    std::thread executionThread(&TaskUsm::executeThread, this);
}

void TaskUsm::transferResult()
{
    isStateChanging_ = true;
    std::thread transferThread(&TaskUsm::transferResultFromDeviceThread, this);
}

IQueue::DeviceResourceGroupId TaskUsm::releaseEventResourceGroup()
{
    event_->releaseResourceGroup();
    eventResourcesAssigned_ = false;
    return eventResourceGroupId_;
}

IQueue::DeviceResourceGroupId TaskUsm::releaseResultResourceGroup()
{
    result_->releaseResourceGroup();
    resultResourcesAssigned_ = false;
    return resultResourceGroupId_;
}

void TaskUsm::setState(State state)
{
    state_ = state;
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
    ITask::ExecutionEvents executionEvents = executeOnDevice(syclQueue);
    queue_->checkinQueue();

    for (auto& executionEvent : executionEvents)
    {
        executionEvent->wait();
    }

    isStateChanging_ = false;
    setState(State::Executed);
}

void TaskUsm::transferResultFromDeviceThread()
{
    sycl::queue& syclQueue = queue_->checkoutQueue();
    DataUsm::TransferResults transferResults = result_->transferToHost(syclQueue);
    queue_->checkinQueue();

    for (auto& transferResult : transferResults)
    {
        transferResult->wait();
    }

    isStateChanging_ = false;
    setState(State::Completed);
}