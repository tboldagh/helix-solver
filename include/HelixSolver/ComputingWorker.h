#pragma once

#include "HelixSolver/EventBuffer.h"
#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/ProcessingQueue.h"
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

        enum class Platform
        {
            BAD_PLATFORM=0,
            CPU_NO_SYCL=1,
            CPU=2,
            GPU=3,
            FPGA=4,
            FPGA_EMULATOR=5
        };

        using EventSoutionsPair = std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>;

        ComputingWorkerState updateAndGetState();
        void setState(ComputingWorkerState state);
        bool assignBuffer(std::shared_ptr<EventBuffer> eventBuffer);
        ComputingWorker(std::unique_ptr<Queue>&& queue);
        const Queue* getQueue() const;

        EventSoutionsPair transferSolutions();
        void waitUntillCompleted();

    private:
        void updateState();
        void scheduleTasksToQueue();

        ComputingWorkerState state = ComputingWorkerState::WAITING;
        std::shared_ptr<EventBuffer> eventBuffer = nullptr; // ?
        std::unique_ptr<std::vector<SolutionCircle>> solutions;
        std::unique_ptr<SolutionBuffer> solutionsBuffer;
        std::unique_ptr<Queue> queue;

#ifdef USE_SYCL
        sycl::event computingEvent;
#endif        

    };
} // HelixSolver
