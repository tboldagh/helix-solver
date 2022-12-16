#pragma once

#include <queue>

#include "HelixSolver/ComputingWorker.h"

namespace HelixSolver
{
    class ComputingManager
    {
    public:
        ComputingManager(ComputingWorker::Platform platform, uint32_t numBuffers, uint32_t numWorkers);

        bool addEvent(std::shared_ptr<Event> event);
        void waitUntillAllTasksCompleted();
        void waitForWaitingWorker();
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> transferSolutions();
        void update();

    private:
        void startProcessingReadyBuffers();
        void transferSolutionsFromCompletedWorkers();
        std::unique_ptr<sycl::queue> getNewQueue() const;

        ComputingWorker::Platform platform;
        std::vector<std::shared_ptr<EventBuffer>> eventBuffers;
        std::vector<std::shared_ptr<ComputingWorker>> computingWorkers;
        std::unique_ptr<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>> solutions;
        std::queue<uint32_t> freeEventBuffers;
        std::queue<uint32_t> readyEventBuffers;
        std::vector<uint32_t> processedEventBuffers;
        std::queue<uint32_t> waitingComputingWorkers;
        std::vector<uint32_t> processingComputingWorkers;
    };
} // namespace HelixSolver
