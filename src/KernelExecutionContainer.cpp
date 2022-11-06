#include <iostream>
#include <limits>
#include <cmath>
#include <fstream>

#include "HelixSolver/HoughTransformKernel.h"
#include "HelixSolver/KernelExecutionContainer.h"
#include "HelixSolver/AccumulatorHelper.h"

namespace HelixSolver
{
    KernelExecutionContainer::KernelExecutionContainer(nlohmann::json& config, Event& event)
    : config(config["main_accumulator_config"])
    , event(event)
    {
        std::string platformStr = config["platform"].get<std::string>();
        platform = platformStr == "cpu" ? Platform::CPU
                : platformStr == "gpu" ? Platform::GPU
                : platformStr == "fpga" ? Platform::FPGA
                : platformStr == "fpga_emulator" ? Platform::FPGA_EMULATOR
                : platform = Platform::BAD_PLATFORM;
        if (platform == Platform::BAD_PLATFORM)
        {
            // TODO: Add BAD_PLATFORM handling
            return;
        }

        map.fill(SolutionCircle{});
    }

    void KernelExecutionContainer::fillOnDevice()
    {
        std::unique_ptr<sycl::queue> queue(getQueue());

        printInfo(queue);

        std::array<SolutionCircle, ACC_SIZE> tempMap;
        tempMap.fill(SolutionCircle{});
        sycl::buffer<SolutionCircle, 1> mapBuffer(tempMap.begin(), tempMap.end());

        sycl::buffer<float, 1> rBuffer(event.getR().begin(), event.getR().end());
        sycl::buffer<float, 1> phiBuffer(event.getPhi().begin(), event.getPhi().end());

        #ifdef DEBUG
        std::vector<uint8_t> accumulatorSum(ACC_SIZE, 0);
        sycl::buffer<uint8_t, 1> accumulatorSumBuf(accumulatorSum.begin(), accumulatorSum.end());
        #endif

        sycl::event qEvent = queue->submit([&](sycl::handler &handler)
        {
            #ifdef DEBUG
            HoughTransformKernel kernel(handler, mapBuffer, rBuffer, phiBuffer, accumulatorSumBuf);
            #else
            HoughTransformKernel kernel(handler, mapBuffer, rBuffer, phiBuffer);
            #endif

            handler.single_task<HoughTransformKernel>(kernel);
        });

        #ifdef DEBUG
        sycl::host_accessor sumBufAccessor(accumulatorSumBuf, sycl::read_only);
        for (uint32_t i = 0; i < ACC_SIZE; i++) accumulatorSum[i] = sumBufAccessor[i];
        
        try
        {
            std::ofstream logFile(ACCUMULATOR_DUMP_FILE_PATH);
            for (uint32_t phi = 0; phi < ACC_HEIGHT; phi++)
            {
                for (uint32_t qOverPt = 0; qOverPt < ACC_WIDTH; qOverPt++) logFile << int(accumulatorSum[phi * ACC_WIDTH + qOverPt]) << " ";
                logFile<<"\n";
            }
        }
        catch (std::exception &exc)
        {
            std::cerr << exc.what() << std::endl;
            exit(EXIT_FAILURE);
        }
        #endif

        qEvent.wait();

        sycl::cl_ulong kernelStartTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_start>();
        sycl::cl_ulong kernelEndTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_end>();
        double kernelTime = (kernelEndTime - kernelStartTime) / NS;

        std::cout << "Execution time: " << kernelTime << " seconds" << std::endl;

        sycl::host_accessor hostMapAccessor(mapBuffer, sycl::read_only);
        for (uint32_t i = 0; i < ACC_SIZE; ++i) map[i] = hostMapAccessor[i];
    }

    void KernelExecutionContainer::printInfo(const std::unique_ptr<sycl::queue>& queue) const 
    {
        sycl::platform platform = queue->get_context().get_platform();
        sycl::device device = queue->get_device();
        std::cout << "Platform: " <<  platform.get_info<sycl::info::platform::name>().c_str() << std::endl;
        std::cout << "Device: " <<  device.get_info<sycl::info::device::name>().c_str() << std::endl;
    }

    const std::array<SolutionCircle, ACC_SIZE> &KernelExecutionContainer::getSolution() const
    {
        return map;
    }

    sycl::queue* KernelExecutionContainer::getQueue()
    {
        sycl::property_list propertyList = sycl::property_list{sycl::property::queue::enable_profiling()};
        
        switch (platform)
        {
            case Platform::BAD_PLATFORM:
                return nullptr;
            case Platform::CPU:
                return new sycl::queue(sycl::host_selector{}, NULL, propertyList);
            case Platform::GPU:
                return new sycl::queue(sycl::gpu_selector{}, NULL, propertyList);
            case Platform::FPGA:
                return new sycl::queue(sycl::ext::intel::fpga_selector{}, NULL, propertyList);
            case Platform::FPGA_EMULATOR:
                return new sycl::queue(sycl::ext::intel::fpga_emulator_selector{}, NULL, propertyList);
        }
    }

    void KernelExecutionContainer::printMainAcc() const
    {
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i)
        {
            for (uint32_t j = 0; j < ACC_WIDTH; ++j) std::cout << int(map[i * ACC_WIDTH + j].isValid) << " ";
            std::cout << std::endl;
        }
    }
} // namespace HelixSolver
