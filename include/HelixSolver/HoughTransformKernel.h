#pragma once

#include <CL/sycl.hpp>
#include <stack>

#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"

#include "HelixSolver/Debug.h"

namespace HelixSolver
{
    class HoughTransformKernel
    {
    private:
        class HoughTransformKernelAccumulatorSection
        {
        public:
            HoughTransformKernelAccumulatorSection() = default;
            HoughTransformKernelAccumulatorSection(uint8_t qOverPtGridDivisionLevel, uint8_t phiGridDivisionLevel, uint16_t qOverPtBeginIndex, uint16_t phiBeginIndex);

            uint8_t qOverPtGridDivisionLevel;
            uint8_t phiGridDivisionLevel;
            uint16_t qOverPtBeginIndex;
            uint16_t phiBeginIndex;
        };
        
    public:
        #ifdef DEBUG
        HoughTransformKernel(sycl::handler& h,
                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                            sycl::buffer<float, 1>& rBuffer,
                            sycl::buffer<float, 1>& phiBuffer,
                            sycl::buffer<uint8_t, 1>& accumulatorSumBuf);
        #else
        HoughTransformKernel(sycl::handler& h,
                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                            sycl::buffer<float, 1>& rBuffer,
                            sycl::buffer<float, 1>& phiBuffer);
        #endif

        SYCL_EXTERNAL void operator()() const;

    private:
        void fillAccumulator(float* rs, float* phis, uint8_t* accumulator) const;

        void fillAccumulatorAdaptive(uint8_t* accumulator) const;

        void fillAccumulatorSectionAdaptive(HoughTransformKernelAccumulatorSection* sectionsStack, uint8_t& sectionsStackHeight, uint8_t* accumulator, uint32_t* stubIndexes, uint32_t* stubCounts) const;

        void fillHits(uint32_t* stubIndexes, uint32_t* stubCounts, uint8_t divisionLevel, const HoughTransformKernelAccumulatorSection& section) const;

        void transferSolutionToHostDevice(uint8_t* accumulator) const;

        static uint32_t mapToBeanIndex(float y);

        // TODO: remove hungarian notation
        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> m_mapAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> m_rAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> m_phiAccessor;
        #ifdef DEBUG
        sycl::accessor<uint8_t, 1, sycl::access::mode::write, sycl::access::target::device> accumulatorSumBuf;
        #endif
    };
} // namespace HelixSolver
