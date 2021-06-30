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

    uint32_t getNearest_FPGA(float *arr, uint32_t x, uint32_t y, float target) {
        uint32_t idx = x;
        if (target - arr[x] >= arr[y] - target)
            idx = y;
        
        return idx;
    }

    uint32_t FindClosest_FPGA(float *arr, uint32_t n, float target) {
        if (target <= arr[0])
            return 0;
        if (target >= arr[n - 1])
            return n - 1;
        uint32_t left = 0, right = n, mid = 0;
        while (left < right) {
            mid = (left + right) / 2;
            if (arr[mid] == target)
                return mid;
            if (target < arr[mid]) {
                if (mid > 0 && target > arr[mid - 1])
                {
                    return getNearest_FPGA(arr, mid - 1, mid, target);
                }
                    
                right = mid;
            } else {
                if (mid < n - 1 && target < arr[mid + 1])
                {
                    return getNearest_FPGA(arr, mid, mid + 1, target);
                }
                left = mid + 1;
            }
        }
        return mid;
    }


    uint32_t FindBeanIdx_FPGA(float y) {
        constexpr float temp = (ACC_HEIGHT - 1) / (PHI_END - PHI_BEGIN);
        float x = temp * (y - PHI_BEGIN);
        return floor(x + 0.5);
    }

    Accumulator::Accumulator(nlohmann::json &p_config, const Event &m_event)
            : m_config(p_config), m_event(m_event) {
        PrepareLinspaces();

        m_dx = m_X[1] - m_X[0];
        m_dxHalf = m_dx / 2;

        m_dy = m_Y[1] - m_Y[0];

        m_map.fill(0);
    }

    void Accumulator::FillOnDevice() {
        sycl::INTEL::fpga_emulator_selector device_selector;
        auto propertyList = sycl::property_list{sycl::property::queue::enable_profiling()};
        sycl::queue fpgaQueue(device_selector, NULL, propertyList);

        sycl::platform platform = fpgaQueue.get_context().get_platform();
        sycl::device device = fpgaQueue.get_device();
        std::cout << "Platform: " <<  platform.get_info<sycl::info::platform::name>().c_str() << std::endl;
        std::cout << "Device: " <<  device.get_info<sycl::info::device::name>().c_str() << std::endl;

        std::array<uint8_t, ACC_SIZE> tempMap;
        tempMap.fill(0);
        sycl::buffer<uint8_t, 1> mapBuffer(tempMap.begin(), tempMap.end());

        std::vector<float> rVec = m_event.GetR();
        std::vector<float> phiVec = m_event.GetPhi();

        sycl::buffer<float, 1> rBuffer(rVec.begin(), rVec.end());
        sycl::buffer<float, 1> phiBuffer(phiVec.begin(), phiVec.end());
        
        sycl::buffer<float, 1> XLinspaceBuf(m_X.begin(), m_X.end());
        sycl::buffer<float, 1> YLinspaceBuf(m_Y.begin(), m_Y.end());

        sycl::event qEvent = fpgaQueue.submit([&](sycl::handler &h) {

            sycl::accessor mapAccessor(mapBuffer, h, sycl::write_only);
            
            sycl::accessor rAccessor(rBuffer, h, sycl::read_only);
            sycl::accessor phiAccessor(phiBuffer, h, sycl::read_only);

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
                uint8_t ACCUMULATOR[ACC_SIZE];

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
                    }
                }

                float dx = X[1] - X[0];
                float dxHalf = dx / 2.0;

                float x, xLeft, xRight, yLeft, yRight, yLeftIdx, yRightIdx;

                for (uint32_t j = 0; j < stubsNum; ++j) {
                    // #pragma unroll 128
                    // [[intel::ivdep]]
                    for (uint32_t i = 0; i < ACC_WIDTH; ++i) {
                        x = X[i];
                        xLeft = x - dxHalf;
                        xRight = x + dxHalf;

                        yLeft = calcAngle_FPGA(R[j], xLeft, PHI[j]);
                        yRight = calcAngle_FPGA(R[j], xRight, PHI[j]);
                        
                        // yLeftIdx = FindClosest_FPGA(Y, ACC_HEIGHT, yLeft);
                        // yRightIdx = FindClosest_FPGA(Y, ACC_HEIGHT, yRight);

                        yLeftIdx = FindBeanIdx_FPGA(yLeft);
                        yRightIdx = FindBeanIdx_FPGA(yRight);

                        // #pragma unroll 128
                        // [[intel::ivdep]]
                        for (uint32_t k = 0; k < ACC_HEIGHT; ++k) {
                            uint8_t incrementValue = 0;
                            if (k >= yRightIdx && k <= yLeftIdx) {
                                incrementValue = 1;
                            }
                            ACCUMULATOR[k * ACC_WIDTH + i] += incrementValue;
                        }
                    }
                }

                // #pragma unroll 64
                // [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_SIZE; ++i) {
                    mapAccessor[i] = ACCUMULATOR[i];
                }
            });
        });

        qEvent.wait();

        sycl::cl_ulong kernelStartTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_start>();
        sycl::cl_ulong kernelEndTime = qEvent.get_profiling_info<sycl::info::event_profiling::command_end>();
        double kernelTime = (kernelEndTime - kernelStartTime) / NS;

        std::cout << "Execution time: " << kernelTime << " seconds" << std::endl;

        sycl::host_accessor hostMapAccessor(mapBuffer, sycl::read_only);
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
                    m_map[j * ACC_WIDTH + i] += 1;
                }
            }
        }
    }

    VectorIdxPair Accumulator::GetCellsAboveThreshold(uint8_t p_threshold) const {
        VectorIdxPair cellsAboveThreshold;
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i) {
            for (uint32_t j = 0; j < ACC_WIDTH; ++j) {
                if (m_map[i * ACC_WIDTH + j] >= p_threshold)
                    cellsAboveThreshold.push_back(std::make_pair(j, i));
            }
        }
        return cellsAboveThreshold;
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
                std::cout << int(m_map[i * ACC_WIDTH + j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    std::pair<double, double> Accumulator::GetValuesOfIndexes(uint32_t x, uint32_t y) const {
        return std::pair<double, double>(m_X[x], m_Y[y]);
    }

} // namespace HelixSolver
