#include <iostream>
#include <limits>
#include <cmath>
#include <fstream>

#include "HelixSolver/HoughTransformKernel.h"
#include "HelixSolver/KernelExecutionContainer.h"
#include "HelixSolver/AccumulatorHelper.h"

namespace HelixSolver
{
    KernelExecutionContainer::KernelExecutionContainer(nlohmann::json& p_config, Event& event)
    : config(p_config)
    , event(event)
    {
        map.fill(SolutionCircle{});
    }

    void KernelExecutionContainer::fillOnDevice()
    {
        std::unique_ptr<sycl::queue> fpgaQueue(getFpgaQueue());

        printInfo(fpgaQueue);

        std::array<SolutionCircle, ACC_SIZE> tempMap;
        tempMap.fill(SolutionCircle{});
        sycl::buffer<SolutionCircle, 1> mapBuffer(tempMap.begin(), tempMap.end());

        sycl::buffer<float, 1> rBuffer(event.getR().begin(), event.getR().end());
        sycl::buffer<float, 1> phiBuffer(event.getPhi().begin(), event.getPhi().end());

        #ifdef DEBUG
        std::vector<uint8_t> accumulatorSum(ACC_SIZE, 0);
        sycl::buffer<uint8_t, 1> accumulatorSumBuf(accumulatorSum.begin(), accumulatorSum.end());
        #endif

        sycl::event qEvent = fpgaQueue->submit([&](sycl::handler &handler)
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

    void KernelExecutionContainer::fill()
    {
        std::vector<float> xLinespace;
        std::vector<float> yLinespace;
        linspace(xLinespace, Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH);
        linspace(yLinespace, PHI_BEGIN, PHI_END, ACC_HEIGHT);
        double dx = xLinespace[1] - xLinespace[0];
        double dxHalf = dx / 2;

        for (const auto& stubFunc : event.getStubsFuncs())
        {
            for (uint32_t i = 0; i < xLinespace.size(); ++i)
            {
                float x = xLinespace[i];
                float xLeft = x - dxHalf;
                float xRight = x + dxHalf;
                
                float yLeft = stubFunc(xLeft);
                float yRight = stubFunc(xRight);

                float yLeftIdx = findClosest(yLinespace, yLeft);
                float yRightIdx = findClosest(yLinespace, yRight);

                for (uint32_t j = yRightIdx; j <= yLeftIdx; ++j)
                {
                    map[j * ACC_WIDTH + i].isValid = true;
                }
            }
        }
    }

    const std::array<SolutionCircle, ACC_SIZE> &KernelExecutionContainer::getSolution() const
    {
        return map;
    }

    sycl::queue* KernelExecutionContainer::getFpgaQueue()
    {
        #if defined(DEBUG)
            sycl::host_selector device_selector;
        #elif defined(FPGA_EMULATOR)
            sycl::ext::intel::fpga_emulator_selector device_selector;
        #else
            sycl::ext::intel::fpga_selector device_selector;
        #endif

        auto propertyList = sycl::property_list{sycl::property::queue::enable_profiling()};
        return new sycl::queue(device_selector, NULL, propertyList);
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
