#include "HelixSolver/ComputingManager.h"
#include "HelixSolver/Debug.h"
#include <stdexcept>
#include <string>

#ifdef USE_SYCL
#include <sycl/ext/intel/fpga_extensions.hpp>
#endif
namespace HelixSolver
{
    ComputingManager::ComputingManager(ComputingWorker::Platform platform, uint32_t numBuffers, uint32_t numWorkers)
    : platform(platform)
    {
        for(uint32_t i = 0; i < numBuffers; i++)
        {
            eventBuffers.push_back(std::make_shared<EventBuffer>());
            freeEventBuffers.push(i);
        }

        for(uint32_t i = 0; i < numWorkers; i++)
        {
            computingWorkers.push_back(std::make_shared<ComputingWorker>(getNewQueue()));
            waitingComputingWorkers.push(i);
        }
        #ifdef USE_SYCL
        // * Expects at leas one worker
        sycl::platform syclPlatform = computingWorkers[0]->getQueue()->get_context().get_platform();
        INFO( "Platform: " <<  syclPlatform.get_info<sycl::info::platform::name>().c_str() );

        // * Expects at leas one worker
        sycl::device device = computingWorkers[0]->getQueue()->get_device();
        INFO( "Device: " <<  device.get_info<sycl::info::device::name>().c_str() );
        #endif
    }

    bool ComputingManager::addEvent(std::shared_ptr<Event> event)
    {
        if(freeEventBuffers.empty()) return false;

        std::shared_ptr<EventBuffer> eventBuffer = eventBuffers[freeEventBuffers.front()];
        eventBuffer->loadEvent(std::move(event));
        eventBuffer->setState(EventBuffer::EventBufferState::READY);
        readyEventBuffers.push(freeEventBuffers.front());
        freeEventBuffers.pop();

        update();

        return true;
    }

    void ComputingManager::waitUntillAllTasksCompleted()
    {
        while(!(readyEventBuffers.empty() && processedEventBuffers.empty() && processingComputingWorkers.empty()))
        {
            while(!processingComputingWorkers.empty())
            {
                computingWorkers[processingComputingWorkers.front()]->waitUntillCompleted();
                update();
            }
            update();
        }
    }

    void ComputingManager::waitForWaitingWorker()
    {
        if(processingComputingWorkers.empty()) return;

        computingWorkers[processingComputingWorkers.front()]->waitUntillCompleted();
    }


    std::unique_ptr<std::vector<ComputingWorker::EventSoutionsPair>> ComputingManager::transferSolutions()
    {
        update();

        return std::move(solutions);
    }

    void ComputingManager::update()
    {
        startProcessingReadyBuffers();
        transferSolutionsFromCompletedWorkers();
    }

    void ComputingManager::startProcessingReadyBuffers()
    {
        while(!waitingComputingWorkers.empty() && !readyEventBuffers.empty())
        {
            computingWorkers[waitingComputingWorkers.front()]->assignBuffer(eventBuffers[readyEventBuffers.front()]);
            processingComputingWorkers.push_back(waitingComputingWorkers.front());
            waitingComputingWorkers.pop();
            processedEventBuffers.push_back(readyEventBuffers.front());
            readyEventBuffers.pop();
        }
    }

    void ComputingManager::transferSolutionsFromCompletedWorkers()
    {
        std::vector<uint32_t> stillProcessingComputingWorkers;
        std::vector<uint32_t> stillProceessedEventBuffers;

        if(!solutions) solutions = std::make_unique<std::vector<std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>>>>();

        for(uint32_t i = 0; i < processingComputingWorkers.size(); i++)
        {
            uint32_t worker = processingComputingWorkers[i];
            uint32_t buffer = processedEventBuffers[i];
            if(computingWorkers[worker]->updateAndGetState() == ComputingWorker::ComputingWorkerState::PROCESSING)
            {
                stillProcessingComputingWorkers.push_back(worker);
                stillProceessedEventBuffers.push_back(buffer);
                continue;
            }

            std::pair<std::shared_ptr<Event>, std::unique_ptr<std::vector<SolutionCircle>>> newSolutions = computingWorkers[worker]->transferSolutions();
            solutions->push_back(std::move(newSolutions));

            waitingComputingWorkers.push(worker);
            computingWorkers[worker]->setState(ComputingWorker::ComputingWorkerState::WAITING);
            freeEventBuffers.push(buffer);
            eventBuffers[buffer]->setState(EventBuffer::EventBufferState::FREE);
        }

        processingComputingWorkers.swap(stillProcessingComputingWorkers);
        processedEventBuffers.swap(stillProceessedEventBuffers);
    }
    std::unique_ptr<Queue> ComputingManager::getNewQueue() const
    {
#ifdef USE_SYCL
        sycl::property_list propertyList = sycl::property_list{sycl::property::queue::enable_profiling()};
        
        switch (platform)
        {
            case ComputingWorker::Platform::CPU:
                return std::make_unique<sycl::queue>(sycl::queue(sycl::host_selector{}, NULL, propertyList));
            case ComputingWorker::Platform::GPU:
                return std::make_unique<sycl::queue>(sycl::queue(sycl::gpu_selector{}, NULL, propertyList));
            case ComputingWorker::Platform::FPGA:
                return std::make_unique<sycl::queue>(sycl::queue(sycl::ext::intel::fpga_selector{}, NULL, propertyList));
            case ComputingWorker::Platform::FPGA_EMULATOR:
                return std::make_unique<sycl::queue>(sycl::queue(sycl::ext::intel::fpga_emulator_selector{}, NULL, propertyList));
            default:
                throw std::runtime_error("Bad platform" + std::to_string(static_cast<int>(platform)) + " in " + __FILE__);
        }
#else        
        switch (platform)
        {
            case ComputingWorker::Platform::CPU_NO_SYCL:
                return std::make_unique<Queue>();
            default:
                throw std::runtime_error("Bad platform: " + std::to_string(static_cast<int>(platform))+ " in " + __FILE__);
        }
#endif
    }
} // namespace HelixSolver
