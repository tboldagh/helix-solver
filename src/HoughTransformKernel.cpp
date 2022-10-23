#include "HelixSolver/HoughTransformKernel.h"
#include "HelixSolver/AccumulatorHelper.h"

namespace HelixSolver
{
    #ifdef DEBUG
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer,
                                            sycl::buffer<uint8_t, 1>& accumulatorSumBuf) 
    : m_mapAccessor(mapBuffer, h, sycl::write_only)
    , m_rAccessor(rBuffer, h, sycl::read_only)
    , m_phiAccessor(phiBuffer, h, sycl::read_only)
    , accumulatorSumBuf(accumulatorSumBuf, h, sycl::write_only) {}
    #else
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer) 
    : m_mapAccessor(mapBuffer, h, sycl::write_only)
    , m_rAccessor(rBuffer, h, sycl::read_only)
    , m_phiAccessor(phiBuffer, h, sycl::read_only) {}
    #endif
    void HoughTransformKernel::transferDataToBoardMemory(float* rs, float* phis) const
    {
        size_t stubsNum = m_rAccessor.size();

        #pragma unroll 64
        [[intel::ivdep]]
        for (uint32_t i = 0; i < stubsNum; ++i)
        {
            rs[i] = m_rAccessor[i];
            phis[i] = m_phiAccessor[i];
        }
    }

    void HoughTransformKernel::fillAccumulator(float* rs, float* phis, uint8_t* accumulator) const
    {

        float xs[ACC_WIDTH];
        linspace(xs, Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH);

        float deltaX = xs[1] - xs[0];
        uint32_t stubsNum = m_rAccessor.size();

        for(uint32_t stubIndex = 0; stubIndex < stubsNum; stubIndex++)
        {
            // TODO: Check if x is q over pt. Rename x prefix in variable names to qOverPt if so.
            // TODO: Change iteration range so as to cut calculation not affecting cells in the accumulator (when phi not in [PHI_BEGIN, PHI_END])
            for(uint32_t xIndex = 0; xIndex < ACC_WIDTH; xIndex++)
            {
                float x = xs[xIndex];
                float xLeft = x - deltaX * 0.5;
                float xRight = x + deltaX * 0.5;

                float phiTop = -rs[stubIndex] * xLeft + phis[stubIndex];
                float phiBottom = -rs[stubIndex] * xRight + phis[stubIndex];

                // * The if statement probably causes votes at the ends of phi dimension not to be counted. Commented lines are intended to fix the bug
                // TODO: Check if the comment above is true and switch to new solution
                uint32_t phiTopIndex = mapToBeanIndex(phiTop);
                uint32_t phiBottomIndex = mapToBeanIndex(phiBottom);
                if (phiBottomIndex < 0 || phiTopIndex >= ACC_HEIGHT) continue;
                // uint32_t phiTopIndex = phiTop < PHI_END ? mapToBeanIndex(phiTop) : ACC_HEIGHT - 1;
                // uint32_t phiBottomIndex = phiBottm > PHI_BEGIN ? mapToBeanIndex(phiBottom) : 0;

                for(uint32_t phiIndex = phiBottomIndex; phiIndex <= phiTopIndex; phiIndex++) accumulator[phiIndex * ACC_WIDTH + xIndex]++;
            }
        }
    }

    void HoughTransformKernel::transferSolutionToHostDevice(uint8_t* accumulator) const
    {
        #pragma unroll 16
        [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_SIZE; ++i)
        {
            bool isAboveThreshold = accumulator[i] > THRESHOLD;

            #ifdef DEBUG
            accumulatorSumBuf[i] = accumulator[i];
            #endif

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

    // TODO: Rename
    uint32_t HoughTransformKernel::mapToBeanIndex(float y)
    {
        constexpr float temp = (ACC_HEIGHT - 1) / (PHI_END - PHI_BEGIN);
        float x = temp * (y - PHI_BEGIN);
        return static_cast<uint32_t>(x + 0.5);
    }

    void HoughTransformKernel::operator()() const
    {
        [[intel::numbanks(4)]]
        float rs[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        float phis[MAX_STUB_NUM];

        [[intel::numbanks(4)]]
        uint8_t accumulator[ACC_SIZE];

        // ? All the data is alredy in sycl buffers. What is the reason behind duplicating the data in transferDataToBoardMemory?
        // TODO: Check if data duplication is necessary. Skip if possible
        transferDataToBoardMemory(rs, phis);
        fillAccumulator(rs, phis, accumulator);
        // This is not transfering solution. It's the last step in calculating it. Refactor
        transferSolutionToHostDevice(accumulator);
    }

} // namespace HelixSolver
