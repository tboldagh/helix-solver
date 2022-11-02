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
        class AccumulatorSection
        {
        public:
            AccumulatorSection() = default;
            AccumulatorSection(uint8_t qOverPtGridDivisionLevel, uint8_t phiGridDivisionLevel, uint16_t qOverPtBeginIndex, uint16_t phiBeginIndex);

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
        void fillAccumulatorAdaptive() const;

        void fillAccumulatorSectionAdaptive(AccumulatorSection* sectionsStack, uint8_t& sectionsStackHeight, uint32_t* stubIndexes, uint32_t* stubCounts) const;

        void fillHits(uint32_t* stubIndexes, uint32_t* stubCounts, uint8_t divisionLevel, const AccumulatorSection& section) const;

        void transferSolutionToHostDevice() const;

        // TODO: Rename
        void addSolutionCircle(uint32_t qOverPtIndex, uint32_t phiIndex) const;;

        static uint32_t mapToBeanIndex(float y);

        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis;
        #ifdef DEBUG
        sycl::accessor<uint8_t, 1, sycl::access::mode::write, sycl::access::target::device> accumulator;
        #endif
    };
} // namespace HelixSolver
