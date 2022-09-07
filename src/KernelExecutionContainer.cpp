#include <iostream>
#include <limits>
#include <cmath>

#include "HelixSolver/HoughTransformKernel.h"

// #include <CL/sycl/INTEL/fpga_extensions.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>


#include "HelixSolver/KernelExecutionContainer.h"
#include "HelixSolver/AccumulatorHelper.h"


namespace HelixSolver {

    KernelExecutionContainer::KernelExecutionContainer(nlohmann::json &p_config, const Event &event)
            : config(p_config), event(event) {
        PrepareLinspaces();

        dx = xs[1] - xs[0];
        dxHalf = dx / 2;

        dy = ys[1] - ys[0];

        m_map.fill(SolutionCircle{0});
    }

    void KernelExecutionContainer::FillOnDevice() {

        #if defined(FPGA_EMULATOR)
            sycl::intel::fpga_emulator_selector device_selector;
        #else
            sycl::intel::fpga_selector device_selector;
        #endif

        auto propertyList = sycl::property_list{sycl::property::queue::enable_profiling()};
        sycl::queue fpgaQueue(device_selector, NULL, propertyList);

        sycl::platform platform = fpgaQueue.get_context().get_platform();
        sycl::device device = fpgaQueue.get_device();
        std::cout << "Platform: " <<  platform.get_info<sycl::info::platform::name>().c_str() << std::endl;
        std::cout << "Device: " <<  device.get_info<sycl::info::device::name>().c_str() << std::endl;

        std::array<SolutionCircle, ACC_SIZE> tempMap;
        tempMap.fill(SolutionCircle{0});
        sycl::buffer<SolutionCircle, 1> mapBuffer(tempMap.begin(), tempMap.end());

        std::vector<float> rVec = event.GetR();
        std::vector<float> phiVec = event.GetPhi();
        std::vector<uint8_t> layers = event.GetLayers();

        sycl::buffer<float, 1> rBuffer(rVec.begin(), rVec.end());
        sycl::buffer<float, 1> phiBuffer(phiVec.begin(), phiVec.end());
        sycl::buffer<uint8_t, 1> layersBuffer(layers.begin(), layers.end());

        sycl::buffer<float, 1> XLinspaceBuf(xs.begin(), xs.end());
        sycl::buffer<float, 1> YLinspaceBuf(ys.begin(), ys.end());


        sycl::event qEvent = fpgaQueue.submit([&](sycl::handler &h) {

            HoughTransformKernel kernel(h, mapBuffer, rBuffer, phiBuffer, layersBuffer, XLinspaceBuf, YLinspaceBuf);

            h.single_task<HoughTransformKernel>(kernel);
        });

        qEvent.wait();

        sycl::cl_ulong kernelStartTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_start>();
        sycl::cl_ulong kernelEndTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_end>();
        double kernelTime = (kernelEndTime - kernelStartTime) / NS;

        std::cout << "Execution time: " << kernelTime << " seconds" << std::endl;

        sycl::host_accessor hostMapAccessor(mapBuffer, sycl::read_only);
        for (uint32_t i = 0; i < ACC_SIZE; ++i) m_map[i] = hostMapAccessor[i];
    }

    void KernelExecutionContainer::Fill() {
        for (const auto& stubFunc : event.GetStubsFuncs()) {
            for (uint32_t i = 0; i < xs.size(); ++i) {
                float x = xs[i];
                float xLeft = x - dxHalf;
                float xRight = x + dxHalf;
                
                float yLeft = stubFunc(xLeft);
                float yRight = stubFunc(xRight);

                float yLeftIdx = FindClosest(ys, yLeft);
                float yRightIdx = FindClosest(ys, yRight);

                for (uint32_t j = yRightIdx; j <= yLeftIdx; ++j) {
                    m_map[j * ACC_WIDTH + i].isValid = true;
                }
            }
        }
    }

    const std::array<SolutionCircle, ACC_SIZE> &KernelExecutionContainer::GetSolution() const {
        return m_map;
    }

    void KernelExecutionContainer::PrepareLinspaces() {
        linspace(xs,
                 Q_OVER_P_BEGIN,
                 Q_OVER_P_END,
                 ACC_WIDTH);

        linspace(ys,
                 PHI_BEGIN,
                 PHI_END,
                 ACC_HEIGHT);
    }

    void KernelExecutionContainer::PrintMainAcc() const {
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i) {
            for (uint32_t j = 0; j < ACC_WIDTH; ++j) {
                std::cout << int(m_map[i * ACC_WIDTH + j].isValid) << " ";
            }
            std::cout << std::endl;
        }
    }

    std::pair<double, double> KernelExecutionContainer::GetValuesOfIndexes(uint32_t x, uint32_t y) const {
        return std::pair<double, double>(xs[x], ys[y]);
    }

} // namespace HelixSolver
