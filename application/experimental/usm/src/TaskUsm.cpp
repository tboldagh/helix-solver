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

std::unique_ptr<EventUsm> TaskUsm::releaseEvent()
{
    if (event_ == nullptr)
    {
        LOG_WARNING("Task has no event assigned, task id: " + std::to_string(id_));
        return nullptr;
    }

    return std::move(event_);
}

std::unique_ptr<ResultUsm> TaskUsm::releaseResult()
{
    if (result_ == nullptr)
    {
        LOG_WARNING("Task has no result assigned, task id: " + std::to_string(id_));
        return nullptr;
    }

    return std::move(result_);
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

void TaskUsm::takeResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resources)
{
    resourceGroupId_ = resources.first;
    event_->setKernelMemory(static_cast<KernelMemory*>(resources.second.at(DeviceResourceType::EventKernelMemory)));
    result_->setKernelMemory(static_cast<KernelMemory*>(resources.second.at(DeviceResourceType::ResultKernelMemory)));
    resourcesAssigned_ = true;

    setState(State::WaitingForEventTransfer);
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

IQueue::DeviceResourceGroupId TaskUsm::releaseResources()
{
    event_->setKernelMemory(nullptr);
    result_->setKernelMemory(nullptr);
    resourcesAssigned_ = false;

    setState(State::Completed);
    return resourceGroupId_;
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

void TaskUsm::transferEventToDeviceThread()
{
    [[maybe_unused]] sycl::queue& syclQueue = queue_->checkoutQueue();
    TransferableData::TransferEvents transferEvents = event_->transferToDevice();
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
    [[maybe_unused]] sycl::queue& syclQueue = queue_->checkoutQueue();
    TransferableData::TransferEvents transferResults = result_->transferToHost();
    queue_->checkinQueue();

    for (auto& transferResult : transferResults)
    {
        transferResult->wait();
    }

    isStateChanging_ = false;
    setState(State::ResultTransferred);
}