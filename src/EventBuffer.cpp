#include "HelixSolver/EventBuffer.h"

namespace HelixSolver
{
    EventBuffer::EventBufferState EventBuffer::getState() const
    {
        return state;
    }

    void EventBuffer::setState(EventBufferState state)
    {
        this->state = state;
    }

    bool EventBuffer::loadEvent(std::shared_ptr<Event> event)
    {
        if (state != EventBufferState::FREE) return false;

        this->event = event;
        rBuffer = std::make_shared<FloatBuffer>(FloatBuffer(event->getR().begin(), event->getR().end()));
        phiBuffer = std::make_shared<FloatBuffer>(FloatBuffer(event->getPhi().begin(), event->getPhi().end()));

        state = EventBufferState::READY;

        return true;
    }

    std::shared_ptr<Event> EventBuffer::getEvent()
    {
        return event;
    }

    std::shared_ptr<FloatBuffer> EventBuffer::getRBuffer() const
    {
        return rBuffer;
    }

    std::shared_ptr<FloatBuffer> EventBuffer::getPhiBuffer() const
    {
        return phiBuffer;
    }
} // namespace HelixSolver
