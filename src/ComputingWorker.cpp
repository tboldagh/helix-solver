#include <nlohmann/json.hpp>
#include "HelixSolver/ComputingWorker.h"
#include "HelixSolver/AdaptiveHoughGpuKernel.h"
#include "HelixSolver/Options.h"
extern nlohmann::json config;

namespace HelixSolver
{
    ComputingWorker::ComputingWorker(std::unique_ptr<Queue>&& queue)
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

    const Queue* ComputingWorker::getQueue() const
    {
        return queue.get();
    }

    ComputingWorker::EventSoutionsPair ComputingWorker::transferSolutions()
    {
        if (state != ComputingWorkerState::COMPLETED) return std::make_pair(std::shared_ptr<Event>(), std::unique_ptr<std::vector<SolutionCircle>>());
        state = ComputingWorkerState::WAITING;
#ifdef USE_SYCL
        // TODO: Move to scheduleTasksToQueue
        sycl::host_accessor solutionsAccessor(*solutionsBuffer, sycl::read_only);
        for (uint32_t i = 0; i < solutionsAccessor.size(); ++i) (*solutions)[i] = solutionsAccessor[i];
#else
    /// TODO come back to this, maybe no need to make the copy
#endif
        return std::make_pair(eventBuffer->getEvent(), std::move(solutions));
    }

    void ComputingWorker::waitUntillCompleted()
    {
        queue->wait();
    }

    void ComputingWorker::updateState()
    {
#ifdef USE_SYCL
        if (state == ComputingWorkerState::WAITING) return;
        sycl::info::event_command_status status = computingEvent.get_info<sycl::info::event::command_execution_status>();
        state = status == sycl::info::event_command_status::complete ? ComputingWorkerState::COMPLETED : ComputingWorkerState::PROCESSING;
#endif
    }

    void ComputingWorker::scheduleTasksToQueue()
    {
#ifdef USE_SYCL
        solutions = std::make_unique<std::vector<SolutionCircle>>();
        solutions->insert(solutions->begin(), MAX_SOLUTIONS, SolutionCircle{});
        solutionsBuffer = std::make_unique<sycl::buffer<SolutionCircle, 1>>(sycl::buffer<SolutionCircle, 1>(solutions->begin(), solutions->end()));

        computingEvent = queue->submit([&](sycl::handler& handler)
        {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs(*eventBuffer->getRBuffer(), handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis(*eventBuffer->getPhiBuffer(), handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> zs(*eventBuffer->getZBuffer(), handler, sycl::read_only);
            sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions(*solutionsBuffer, handler, sycl::write_only);
            AdaptiveHoughGpuKernel kernel(rs, phis, zs, solutions);
            handler.parallel_for(sycl::range<2>(ADAPTIVE_KERNEL_INITIAL_DIVISIONS, ADAPTIVE_KERNEL_INITIAL_DIVISIONS), kernel);
        });

        state = ComputingWorkerState::PROCESSING;
        eventBuffer->setState(EventBuffer::EventBufferState::PROCESSED);
#else
    // in pure CPU code we do not wait for anything
    solutions = std::make_unique<std::vector<SolutionCircle>>(MAX_SOLUTIONS);
    HelixSolver::Options opt;
    opt.ACC_X_PRECISION = config["phi_precision"];
    opt.ACC_PT_PRECISION = config["pt_precision"]; 
    AdaptiveHoughGpuKernel kernel(opt, *eventBuffer->getRBuffer(), *eventBuffer->getPhiBuffer(), *eventBuffer->getZBuffer(), *solutions);
    for ( uint8_t div1 = 0; div1 < ADAPTIVE_KERNEL_INITIAL_DIVISIONS; ++div1 ) {
        for ( uint8_t div2 = 0; div2 < ADAPTIVE_KERNEL_INITIAL_DIVISIONS; ++div2 ) {
            kernel({div1, div2});
        }
    }
    state = ComputingWorkerState::COMPLETED;
#endif
    }

} // namespace HelixSolver
