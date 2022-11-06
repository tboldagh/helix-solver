#pragma once

#include "HelixSolver/EventBuffer.h"
#include "HelixSolver/SolutionCircle.h"

namespace HelixSolver
{
    class ComputingWorker
    {
    public:
        enum class ComputingWorkerState
        {
            WAITING,
            PROCESSING,
            COMPLETED
        };

        ComputingWorker(std::unique_ptr<sycl::queue>&& queue);
        ComputingWorkerState updateAndGetState();
        void setState(ComputingWorkerState state);
        bool assignBuffer(std::shared_ptr<EventBuffer> eventBuffer);
        std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>> transferSolutions();
        void waitUntillCompleted();

    private:
        void updateState();
        void scheduleTasksToQueue();

        ComputingWorkerState state = ComputingWorkerState::WAITING;
        std::shared_ptr<EventBuffer> eventBuffer = nullptr; // ?
        std::unique_ptr<sycl::queue> queue;
        std::unique_ptr<std::vector<SolutionCircle>> solutions;

        std::unique_ptr<sycl::buffer<SolutionCircle, 1>> solutionsBuffer;
        sycl::event computingEvent;
    };
} // HelixSolver
