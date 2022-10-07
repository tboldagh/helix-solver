#include "HelixSolver/HoughTransformKernel.h"

namespace HelixSolver
{
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer,
                                            sycl::buffer<uint8_t, 1>& layersBuffer,
                                            sycl::buffer<float, 1>& xLinspaceBuf,
                                            sycl::buffer<float, 1>& yLinspaceBuf) 
    : m_mapAccessor(mapBuffer, h, sycl::write_only)
    , m_rAccessor(rBuffer, h, sycl::read_only)
    , m_phiAccessor(phiBuffer, h, sycl::read_only)
    , m_layersAccessor(layersBuffer, h, sycl::read_only)
    , m_xLinspaceAccessor(xLinspaceBuf, h, sycl::read_only)
    , m_yLinspaceAccessor(yLinspaceBuf, h, sycl::read_only) {}

    void HoughTransformKernel::transferDataToBoardMemory(float* xs,
                                                        float* ys,
                                                        float* rs,
                                                        float* phis,
                                                        uint8_t* layers) const
    {
        size_t stubsNum = m_rAccessor.size();

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_WIDTH; ++i) xs[i] = m_xLinspaceAccessor[i];

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i) ys[i] = m_yLinspaceAccessor[i];

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < MAX_STUB_NUM; ++i)
        {
            if (i < stubsNum)
            {
                rs[i] = m_rAccessor[i];
                phis[i] = m_phiAccessor[i];
                layers[i] = m_layersAccessor[i];
            }
        }
    }

    void HoughTransformKernel::fillBoardAccumulator(float* xs,
                                                    float* rs,
                                                    float* phis,
                                                    uint8_t* layers,
                                                    bool accumulator[][ACC_SIZE]) const
    {
        size_t stubsNum = m_rAccessor.size();

        float dx = xs[1] - xs[0];
        float dxHalf = dx / 2.0;

        float x, xLeft, xRight, yLeft, yRight;
        uint32_t yLeftIdx, yRightIdx;

        #pragma unroll
        // ? Does it have any effect? "The ivdep pragma is supported in host code only."
        [[intel::ivdep]]
        for (uint8_t layer = 0; layer < NUM_OF_LAYERS; ++layer)
        {
            for (uint32_t j = 0; j < stubsNum; ++j)
            {
                if (layers[j] != layer) continue; // skip if stub does not belong to layer

                #pragma unroll 22
                // ? Does it have any effect? "The ivdep pragma is supported in host code only."
                [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_WIDTH; ++i)
                {
                    x = xs[i];
                    xLeft = x - dxHalf;
                    xRight = x + dxHalf;

                    yLeft = calculateAngle(rs[j], xLeft, phis[j]);
                    yRight = calculateAngle(rs[j], xRight, phis[j]);

                    yLeftIdx = mapToBeanIndex(yLeft);
                    yRightIdx = mapToBeanIndex(yRight);

                    if (yRightIdx < 0 || yLeftIdx >= ACC_HEIGHT) continue;

                    for (uint32_t k = yRightIdx; k <= yLeftIdx; ++k) accumulator[layer][k * ACC_WIDTH + i] = true;
                }
            }
        }
    }

    void HoughTransformKernel::transferSolutionToHostDevice(bool accumulator[][ACC_SIZE]) const
    {
        #pragma unroll 16
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_SIZE; ++i)
        {
            uint8_t sum = accumulator[0][i] +
                        accumulator[1][i] +
                        accumulator[2][i] +
                        accumulator[3][i] +
                        accumulator[4][i] +
                        accumulator[5][i] +
                        accumulator[6][i] +
                        accumulator[7][i];
            bool isAboveThreshold = sum > THRESHOLD;

            if (isAboveThreshold)
            {
                constexpr float qOverPtMultiplier = (Q_OVER_P_END - Q_OVER_P_BEGIN) / ACC_WIDTH;
                constexpr float phiMultiplier = (PHI_END - PHI_BEGIN) / ACC_HEIGHT;

                uint32_t qOverPtIdx = i % ACC_WIDTH;
                uint32_t phiIdx = i / ACC_WIDTH;

                float qOverPt = qOverPtIdx * qOverPtMultiplier + Q_OVER_P_BEGIN;
                float phi_0 = phiIdx * phiMultiplier + PHI_BEGIN;
                
                m_mapAccessor[i].isValid = true;
                m_mapAccessor[i].r = ((1 / qOverPt) / B) * 1000;
                m_mapAccessor[i].phi = phi_0 + M_PI_2;
            }
        }
    }

    float HoughTransformKernel::calculateAngle(float r, float q_over_pt, float phi)
    {
        return -r * q_over_pt + phi;
    };

    uint32_t HoughTransformKernel::mapToBeanIndex(float y)
    {
        constexpr float temp = (ACC_HEIGHT - 1) / (PHI_END - PHI_BEGIN);
        float x = temp * (y - PHI_BEGIN);
        return static_cast<uint32_t>(x + 0.5);
    }

    void HoughTransformKernel::operator()() const
    {
        [[intel::numbanks(4)]]
        float xs[ACC_WIDTH];

        [[intel::numbanks(4)]]
        float ys[ACC_HEIGHT];

        [[intel::numbanks(4)]]
        float rs[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        float phis[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        uint8_t layers[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        bool accumulator[NUM_OF_LAYERS][ACC_SIZE];

        // ? All the data is alredy in sycl buffers. What is the reason behind duplicating the data in transferDataToBoardMemory?
        // TODO: Chck if data duplication is necessary. Skip if possible
        transferDataToBoardMemory(xs, ys, rs, phis, layers);
        fillBoardAccumulator(xs, rs, phis, layers, accumulator);
        // This is not transfering solution. It's the last step in calculating it. Refactor
        transferSolutionToHostDevice(accumulator);
    }

} // namespace HelixSolver
