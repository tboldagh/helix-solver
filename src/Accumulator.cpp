#include <iostream>
#include <limits>
#include <cmath>

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include <HelixSolver/Constants.h>

#include <HelixSolver/Accumulator.h>
#include <HelixSolver/AccumulatorHelper.h>

class KernelHoughTransform;

namespace HelixSolver {

    float calcAngle_FPGA(float r, float x, float phi) {
        return -r * x + phi;
    };


    uint32_t FindBeanIdx_FPGA(float y) {
        constexpr float temp = (ACC_HEIGHT - 1) / (PHI_END - PHI_BEGIN);
        float x = temp * (y - PHI_BEGIN);
        return static_cast<uint32_t>(x + 0.5);
    }

    Accumulator::Accumulator(nlohmann::json &p_config, const Event &m_event)
            : m_config(p_config), m_event(m_event) {
        PrepareLinspaces();

        m_dx = m_X[1] - m_X[0];
        m_dxHalf = m_dx / 2;

        m_dy = m_Y[1] - m_Y[0];

        m_map.fill(SolutionCircle{0});
    }

    void Accumulator::FillOnDevice() {

        #if defined(FPGA_EMULATOR)
            sycl::INTEL::fpga_emulator_selector device_selector;
        #else
            sycl::INTEL::fpga_selector device_selector;
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

            sycl::accessor mapAccessor(mapBuffer, h, sycl::write_only);
            
            sycl::accessor rAccessor(rBuffer, h, sycl::read_only);
            sycl::accessor phiAccessor(phiBuffer, h, sycl::read_only);
            sycl::accessor layersAccessor(layersBuffer, h, sycl::read_only);

            sycl::accessor xLinspaceAccessor(XLinspaceBuf, h, sycl::read_only);
            sycl::accessor yLinspaceAccessor(YLinspaceBuf, h, sycl::read_only);

            h.single_task<class KernelHoughTransform>([=]() [[intel::kernel_args_restrict]] {
                // [[intel::numbanks(8)]]
                float X[ACC_WIDTH];

                // [[intel::numbanks(8)]]
                float Y[ACC_HEIGHT];

                // [[intel::numbanks(8)]]
                float R[MAX_STUB_NUM];

                // [[intel::numbanks(512)]]
                float PHI[MAX_STUB_NUM];

                // [[intel::numbanks(512)]]
                uint8_t LAYER[MAX_STUB_NUM];

                // [[intel::numbanks(512)]]
                bool ACCUMULATOR[NUM_OF_LAYERS][ACC_SIZE];

                // #pragma unroll 128
                // [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_WIDTH; ++i) X[i] = xLinspaceAccessor[i];

                // #pragma unroll 128
                // [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_HEIGHT; ++i) Y[i] = yLinspaceAccessor[i];

                size_t stubsNum = rAccessor.get_count();

                // #pragma unroll 128
                // [[intel::ivdep]]
                for (uint32_t i = 0; i < MAX_STUB_NUM; ++i)
                {
                    if (i < stubsNum) {
                        R[i] = rAccessor[i];
                        PHI[i] = phiAccessor[i];
                        LAYER[i] = layersAccessor[i];
                    }
                }

                float dx = X[1] - X[0];
                float dxHalf = dx / 2.0;

                float x, xLeft, xRight, yLeft, yRight;
                uint32_t yLeftIdx, yRightIdx;

                #pragma unroll
                [[intel::ivdep]]
                for (uint8_t layer = 0; layer < NUM_OF_LAYERS; ++layer) {
                    for (uint32_t j = 0; j < MAX_STUB_NUM; ++j) {
                        if (j >= stubsNum || LAYER[j] != layer) continue; // skip if stub does not belong to layer

                        // #pragma unroll 128
                        // [[intel::ivdep]]
                        for (uint32_t i = 0; i < ACC_WIDTH; ++i) {
                            x = X[i];
                            xLeft = x - dxHalf;
                            xRight = x + dxHalf;

                            yLeft = calcAngle_FPGA(R[j], xLeft, PHI[j]);
                            yRight = calcAngle_FPGA(R[j], xRight, PHI[j]);

                            yLeftIdx = FindBeanIdx_FPGA(yLeft);
                            yRightIdx = FindBeanIdx_FPGA(yRight);

                            // #pragma unroll 128
                            // [[intel::ivdep]]
                            for (uint32_t k = 0; k < ACC_HEIGHT; ++k) {
                                if (k >= yRightIdx && k <= yLeftIdx) {
                                    ACCUMULATOR[layer][k * ACC_WIDTH + i] = true;
                                }
                            }
                        }
                    }
                }

                // #pragma unroll 64
                // [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_SIZE; ++i) {
                    bool belongToAllLayers = true;

                    #pragma unroll
                    [[intel::ivdep]]
                    for (uint8_t j = 0; j < NUM_OF_LAYERS; ++j) {
                        if (!ACCUMULATOR[j][i]) {
                            belongToAllLayers = false; // WHAT WILL HAPPEN ????????????
                        }
                    }

                    if (belongToAllLayers) {
                        constexpr float qOverPtMultiplier = (Q_OVER_P_END - Q_OVER_P_BEGIN) / ACC_WIDTH;
                        constexpr float phiMultiplier = (PHI_END - PHI_BEGIN) / ACC_HEIGHT;

                        uint32_t qOverPtIdx = i % ACC_WIDTH;
                        uint32_t phiIdx = i / ACC_WIDTH;

                        float qOverPt = qOverPtIdx * qOverPtMultiplier + Q_OVER_P_BEGIN;
                        float phi = phiIdx * phiMultiplier + PHI_BEGIN;
                        
                        mapAccessor[i].isValid = true;
                        mapAccessor[i].r = ((1 / qOverPt) / B) * 1000;
                        mapAccessor[i].phi = phi + M_PI_2;
                    }
                }
            });
        });

        qEvent.wait();

        sycl::cl_ulong kernelStartTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_start>();
        sycl::cl_ulong kernelEndTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_end>();
        double kernelTime = (kernelEndTime - kernelStartTime) / NS;

        std::cout << "Execution time: " << kernelTime << " seconds" << std::endl;

        sycl::host_accessor hostMapAccessor(mapBuffer, sycl::read_only);
        // std::copy(hostMapAccessor.begin(), hostMapAccessor.end(), m_map);
        for (uint32_t i = 0; i < ACC_SIZE; ++i) m_map[i] = hostMapAccessor[i];
    }

    void Accumulator::Fill() {
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

    VectorIdxPair Accumulator::GetCellsAboveThreshold(uint8_t p_threshold) const {
        // VectorIdxPair cellsAboveThreshold;
        // for (uint32_t i = 0; i < ACC_HEIGHT; ++i) {
        //     for (uint32_t j = 0; j < ACC_WIDTH; ++j) {
        //         if (m_map[i * ACC_WIDTH + j] >= p_threshold)
        //             cellsAboveThreshold.push_back(std::make_pair(j, i));
        //     }
        // }
        // return cellsAboveThreshold;
    }

    const std::array<SolutionCircle, ACC_SIZE> &Accumulator::GetSolution() const {
        return m_map;
    }

    void Accumulator::PrepareLinspaces() {
        linspace(m_X,
                 Q_OVER_P_BEGIN,
                 Q_OVER_P_END,
                 ACC_WIDTH);

        linspace(m_Y,
                 PHI_BEGIN,
                 PHI_END,
                 ACC_HEIGHT);
    }

    void Accumulator::PrintMainAcc() const {
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i) {
            for (uint32_t j = 0; j < ACC_WIDTH; ++j) {
                std::cout << int(m_map[i * ACC_WIDTH + j].isValid) << " ";
            }
            std::cout << std::endl;
        }
    }

    std::pair<double, double> Accumulator::GetValuesOfIndexes(uint32_t x, uint32_t y) const {
        return std::pair<double, double>(m_X[x], m_Y[y]);
    }

} // namespace HelixSolver
