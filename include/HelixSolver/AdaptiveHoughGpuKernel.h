#pragma once

#include <CL/sycl.hpp>

#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"

#include "HelixSolver/Debug.h"

#include "HelixSolver/EventBuffer.h"

namespace HelixSolver
{
    class AdaptiveHoughGpuKernel
    {
    public:
        AdaptiveHoughGpuKernel(sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs,
                sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis,
                sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solution);

        SYCL_EXTERNAL void operator()(sycl::id<2> idx) const;

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

        void fillAccumulatorSection(AccumulatorSection* sectionsStack, uint8_t& sectionsHeight, uint32_t* stubIndexes, uint32_t* stubCounts) const;
        void fillHits(uint32_t* stubIndexes, uint32_t* stubCounts, uint8_t divisionLevel, const AccumulatorSection& section) const;
        void addSolution(uint32_t qOverPtIndex, uint32_t phiIndex) const;
        static inline float linspaceElement(float start, float end, uint32_t numPoints, uint16_t index);

        static constexpr uint8_t MAX_DIVISION_LEVEL = Q_OVER_PT_MAX_GRID_DIVISION_LEVEL > PHI_MAX_GRID_DIVISION_LEVEL ? Q_OVER_PT_MAX_GRID_DIVISION_LEVEL : PHI_MAX_GRID_DIVISION_LEVEL;

        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis;
        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions;
    };

    inline float AdaptiveHoughGpuKernel::linspaceElement(float start, float end, uint32_t numPoints, uint16_t index)
    {
        return start + (end - start) * index / (numPoints - 1);
    }

} // namespace HelixSolver