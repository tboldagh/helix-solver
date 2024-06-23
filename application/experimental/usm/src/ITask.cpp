#include "EventUsm/ITask.h"


std::string ITask::stateToString(State state)
{
    switch (state)
    {
    case State::Created:
        return "Created";
    case State::EventAndResultAssigned:
        return "EventAndResultAssigned";
    case State::ReadyToQueue:
        return "ReadyToQueue";
    case State::WaitingForResources:
        return "WaitingForResources";
    case State::WaitingForEventTransfer:
        return "WaitingForEventTransfer";
    case State::WaitingForExecution:
        return "WaitingForExecution";
    case State::Executed:
        return "Executed";
    case State::WaitingForResultTransfer:
        return "WaitingForResultTransfer";
    case State::Completed:
        return "Completed";
    default:
        return "Unknown";
    }
}