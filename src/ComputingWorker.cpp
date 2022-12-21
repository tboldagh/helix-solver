#include "HelixSolver/ComputingWorker.h"
#include "HelixSolver/AdaptiveHoughGpuKernel.h"

namespace HelixSolver
{
    ComputingWorker::ComputingWorker(std::unique_ptr<sycl::queue>&& queue)
    : queue(std::move(queue)) {}

    ComputingWorker::ComputingWorkerState ComputingWorker::updateAndGetState()
    {
        updateState();

        return state;
    }

    void ComputingWorker::setState(ComputingWorkerState state)
    {
        this->state = state;
    }

    bool ComputingWorker::assignBuffer(std::shared_ptr<EventBuffer> eventBuffer)
    {
        if (state != ComputingWorkerState::WAITING || eventBuffer->getState() != EventBuffer::EventBufferState::READY) return false;
        
        this->eventBuffer = std::move(eventBuffer);
        scheduleTasksToQueue();

        return true;
    }

    const sycl::queue* ComputingWorker::getQueue() const
    {
        return queue.get();
    }

    std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>> ComputingWorker::transferSolutions()
    {
        if (state != ComputingWorkerState::COMPLETED) return std::make_pair(std::shared_ptr<Event>(), std::unique_ptr<std::vector<SolutionCircle>>());
        state = ComputingWorkerState::WAITING;

        // TODO: Move to scheduleTasksToQueue
        sycl::host_accessor solutionsAccessor(*solutionsBuffer, sycl::read_only);
        for (uint32_t i = 0; i < ACC_SIZE; ++i) (*solutions)[i] = solutionsAccessor[i];

        return std::make_pair(eventBuffer->getEvent(), std::move(solutions));
    }

    void ComputingWorker::waitUntillCompleted()
    {
        queue->wait();
    }

    void ComputingWorker::updateState()
    {
        if (state == ComputingWorkerState::WAITING) return;

        sycl::info::event_command_status status = computingEvent.get_info<sycl::info::event::command_execution_status>();
        state = status == sycl::info::event_command_status::complete ? ComputingWorkerState::COMPLETED : ComputingWorkerState::PROCESSING;
    }

    void ComputingWorker::scheduleTasksToQueue()
    {
        solutions = std::make_unique<std::vector<SolutionCircle>>();
        solutions->insert(solutions->begin(), ACC_SIZE, SolutionCircle{});
        solutionsBuffer = std::make_unique<sycl::buffer<SolutionCircle, 1>>(sycl::buffer<SolutionCircle, 1>(solutions->begin(), solutions->end()));

        computingEvent = queue->submit([&](sycl::handler& handler)
        {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs(*eventBuffer->getRBuffer(), handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis(*eventBuffer->getPhiBuffer(), handler, sycl::read_only);
            sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions(*solutionsBuffer, handler, sycl::write_only);
            AdaptiveHoughGpuKernel kernel(rs, phis, solutions);
            handler.parallel_for(sycl::range<2>(ADAPTIVE_KERNEL_INITIAL_DIVISIONS, ADAPTIVE_KERNEL_INITIAL_DIVISIONS), kernel);
        });
        
        state = ComputingWorkerState::PROCESSING;
        eventBuffer->setState(EventBuffer::EventBufferState::PROCESSED);
    }

} // namespace HelixSolver
