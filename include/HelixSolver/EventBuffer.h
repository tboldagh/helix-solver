# pragma once
#include "Event.h"
#include "SolutionCircle.h"

#ifdef USE_SYCL
    #include <CL/sycl.hpp>
    using FloatBuffer=sycl::buffer<float, 1>;
    using SolutionBuffer=sycl::buffer<HelixSolver::SolutionCircle, 1>;
#else
    #include <vector>
    using FloatBuffer=std::vector<float>;
    using SolutionBuffer=std::vector<HelixSolver::SolutionCircle>;
#endif


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
        std::shared_ptr<FloatBuffer> getRBuffer() const;
        std::shared_ptr<FloatBuffer> getPhiBuffer() const;
        std::shared_ptr<FloatBuffer> getZBuffer() const;

    private:
        EventBufferState state = EventBufferState::FREE;
        std::shared_ptr<Event> event;
        std::shared_ptr<FloatBuffer> rBuffer;
        std::shared_ptr<FloatBuffer> phiBuffer;
        std::shared_ptr<FloatBuffer> zBuffer;
    };
} // namespace HelixSolver
