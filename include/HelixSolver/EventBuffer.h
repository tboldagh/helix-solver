# pragma once

#include <CL/sycl.hpp>

#include "Event.h"

namespace HelixSolver
{
    class EventBuffer
    {
    public:
        enum class EventBufferState
        {
            FREE,
            // LOADING,
            READY,
            PROCESSED
        };

        EventBufferState getState() const;
        void setState(EventBufferState state);
        bool loadEvent(std::shared_ptr<Event> event);
        std::shared_ptr<Event> getEvent();
        std::shared_ptr<sycl::buffer<float, 1>> getRBuffer() const;
        std::shared_ptr<sycl::buffer<float, 1>> getPhiBuffer() const;

    private:
        EventBufferState state = EventBufferState::FREE;
        std::shared_ptr<Event> event;
        std::shared_ptr<sycl::buffer<float, 1>> rBuffer;
        std::shared_ptr<sycl::buffer<float, 1>> phiBuffer;
    };
} // namespace HelixSolver
