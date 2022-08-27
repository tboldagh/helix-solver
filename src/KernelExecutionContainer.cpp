#include <iostream>
#include <limits>
#include <cmath>

#include "HelixSolver/HoughTransformKernel.h"

// #include <CL/sycl/INTEL/fpga_extensions.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>


#include "HelixSolver/KernelExecutionContainer.h"
#include "HelixSolver/AccumulatorHelper.h"


namespace HelixSolver {

    KernelExecutionContainer::KernelExecutionContainer(nlohmann::json &p_config, const Event &m_event)
            : m_config(p_config), m_event(m_event) {
        PrepareLinspaces();

        m_dx = m_X[1] - m_X[0];
        m_dxHalf = m_dx / 2;

        m_dy = m_Y[1] - m_Y[0];

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

        std::vector<float> rVec = m_event.GetR();
        std::vector<float> phiVec = m_event.GetPhi();
        std::vector<uint8_t> layers = m_event.GetLayers();

        sycl::buffer<float, 1> rBuffer(rVec.begin(), rVec.end());
        sycl::buffer<float, 1> phiBuffer(phiVec.begin(), phiVec.end());
        sycl::buffer<uint8_t, 1> layersBuffer(layers.begin(), layers.end());

        sycl::buffer<float, 1> XLinspaceBuf(m_X.begin(), m_X.end());
        sycl::buffer<float, 1> YLinspaceBuf(m_Y.begin(), m_Y.end());


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
        for (const auto& stubFunc : m_event.GetStubsFuncs()) {
            for (uint32_t i = 0; i < m_X.size(); ++i) {
                float x = m_X[i];
                float xLeft = x - m_dxHalf;
                float xRight = x + m_dxHalf;
                
                float yLeft = stubFunc(xLeft);
                float yRight = stubFunc(xRight);

                float yLeftIdx = FindClosest(m_Y, yLeft);
                float yRightIdx = FindClosest(m_Y, yRight);

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
        linspace(m_X,
                 Q_OVER_P_BEGIN,
                 Q_OVER_P_END,
                 ACC_WIDTH);

        linspace(m_Y,
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
        return std::pair<double, double>(m_X[x], m_Y[y]);
    }

} // namespace HelixSolver
