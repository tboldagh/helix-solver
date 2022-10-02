#include "HelixSolver/HoughTransformKernel.h"

namespace HelixSolver
{
    HoughTransformKernel::HoughTransformKernel(sycl::handler &h,
                                            sycl::buffer<SolutionCircle, 1> &mapBuffer,
                                            sycl::buffer<float, 1> &rBuffer,
                                            sycl::buffer<float, 1> &phiBuffer,
                                            sycl::buffer<uint8_t, 1> &layersBuffer,
                                            sycl::buffer<float, 1> &XLinspaceBuf,
                                            sycl::buffer<float, 1> &YLinspaceBuf) 
    : m_mapAccessor(mapBuffer, h, sycl::write_only)
    , m_rAccessor(rBuffer, h, sycl::read_only)
    , m_phiAccessor(phiBuffer, h, sycl::read_only)
    , m_layersAccessor(layersBuffer, h, sycl::read_only)
    , m_xLinspaceAccessor(XLinspaceBuf, h, sycl::read_only)
    , m_yLinspaceAccessor(YLinspaceBuf, h, sycl::read_only) {}

    void HoughTransformKernel::transferDataToBoardMemory(float* x,
                                                        float* y,
                                                        float* r,
                                                        float* phi,
                                                        uint8_t* layer) const
    {
        size_t stubsNum = m_rAccessor.get_count();

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_WIDTH; ++i) x[i] = m_xLinspaceAccessor[i];

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_HEIGHT; ++i) y[i] = m_yLinspaceAccessor[i];

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < MAX_STUB_NUM; ++i)
        {
            if (i < stubsNum)
            {
                r[i] = m_rAccessor[i];
                phi[i] = m_phiAccessor[i];
                layer[i] = m_layersAccessor[i];
            }
        }
    }

    void HoughTransformKernel::fillBoardAccumulator(float *X,
                                                    float *R,
                                                    float *PHI,
                                                    uint8_t *LAYER,
                                                    bool ACCUMULATOR[][ACC_SIZE]) const
    {
        size_t stubsNum = m_rAccessor.get_count();

        float dx = X[1] - X[0];
        float dxHalf = dx / 2.0;

        float x, xLeft, xRight, yLeft, yRight;
        uint32_t yLeftIdx, yRightIdx;

        #pragma unroll
        // ? Does it have any effect? "The ivdep pragma is supported in host code only."
        [[intel::ivdep]]
        for (uint8_t layer = 0; layer < NUM_OF_LAYERS; ++layer)
        {
            for (uint32_t j = 0; j < MAX_STUB_NUM; ++j)
            {
                if (j >= stubsNum || LAYER[j] != layer) continue; // skip if stub does not belong to layer

                #pragma unroll 22
                // ? Does it have any effect? "The ivdep pragma is supported in host code only."
                [[intel::ivdep]]
                for (uint32_t i = 0; i < ACC_WIDTH; ++i)
                {
                    x = X[i];
                    xLeft = x - dxHalf;
                    xRight = x + dxHalf;

                    yLeft = calculateAngle(R[j], xLeft, PHI[j]);
                    yRight = calculateAngle(R[j], xRight, PHI[j]);

                    yLeftIdx = mapToBeanIndex(yLeft);
                    yRightIdx = mapToBeanIndex(yRight);

                    if (yRightIdx < 0 || yLeftIdx >= ACC_HEIGHT) continue;

                    for (uint32_t k = yRightIdx; k <= yLeftIdx; ++k) ACCUMULATOR[layer][k * ACC_WIDTH + i] = true;
                }
            }
        }
    }

    void HoughTransformKernel::transferSolutionToHostDevice(bool ACCUMULATOR[][ACC_SIZE]) const
    {
        #pragma unroll 16
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_SIZE; ++i)
        {
            uint8_t sum = ACCUMULATOR[0][i] +
                        ACCUMULATOR[1][i] +
                        ACCUMULATOR[2][i] +
                        ACCUMULATOR[3][i] +
                        ACCUMULATOR[4][i] +
                        ACCUMULATOR[5][i] +
                        ACCUMULATOR[6][i] +
                        ACCUMULATOR[7][i];
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
        float X[ACC_WIDTH];

        [[intel::numbanks(4)]]
        float Y[ACC_HEIGHT];

        [[intel::numbanks(4)]]
        float R[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        float PHI[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        uint8_t LAYER[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        bool ACCUMULATOR[NUM_OF_LAYERS][ACC_SIZE];

        transferDataToBoardMemory(X, Y, R, PHI, LAYER);
        fillBoardAccumulator(X, R, PHI, LAYER, ACCUMULATOR);
        transferSolutionToHostDevice(ACCUMULATOR);
    }

} // namespace HelixSolver
