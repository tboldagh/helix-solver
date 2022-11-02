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
            AccumulatorSection(uint16_t width, uint16_t height, uint16_t xBegin, uint16_t yBegin);

            uint16_t width;
            uint16_t height;
            uint16_t xBegin;
            uint16_t yBegin;
        };
        
    public:
        #ifdef DEBUG
        HoughTransformKernel(sycl::handler& h,
                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                            sycl::buffer<float, 1>& rBuffer,
                            sycl::buffer<float, 1>& phiBuffer,
                            sycl::buffer<uint8_t, 1>& accumulatorBuffer);
        #else
        HoughTransformKernel(sycl::handler& h,
                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                            sycl::buffer<float, 1>& rBuffer,
                            sycl::buffer<float, 1>& phiBuffer);
        #endif

        SYCL_EXTERNAL void operator()() const;

    private:
        void fillAccumulator() const;

        void fillAccumulatorSection(AccumulatorSection* sectionsStack, uint8_t& sectionsHeight, uint32_t* stubIndexes, uint32_t* stubCounts) const;

        void fillHits(uint32_t* stubIndexes, uint32_t* stubCounts, uint8_t divisionLevel, const AccumulatorSection& section) const;

        void transferSolutionToHostDevice() const;

        // TODO: Rename
        void addSolutionCircle(uint32_t qOverPtIndex, uint32_t phiIndex) const;;

        static constexpr uint8_t MAX_DIVISION_LEVEL = Q_OVER_PT_MAX_GRID_DIVISION_LEVEL > PHI_MAX_GRID_DIVISION_LEVEL ? Q_OVER_PT_MAX_GRID_DIVISION_LEVEL : PHI_MAX_GRID_DIVISION_LEVEL;

        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis;
        #ifdef DEBUG
        sycl::accessor<uint8_t, 1, sycl::access::mode::write, sycl::access::target::device> accumulator;
        #endif
    };
} // namespace HelixSolver
