#pragma once

#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/Debug.h"
#include "HelixSolver/EventBuffer.h"

#ifdef USE_SYCL
#include <CL/sycl.hpp>
using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
using SolutionsWriteAccessor = sycl::accessor<HelixSolver::SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device>;
using Index2D = sycl::id<2>;
#else
#include <vector>
#include <array>
using FloatBufferReadAccessor = const FloatBuffer &;
using SolutionsWriteAccessor = std::vector<HelixSolver::SolutionCircle> &;
using Index2D = std::array<int, 2>;
#define SYCL_EXTERNAL
#endif

namespace HelixSolver
{
    class AdaptiveHoughGpuKernel
    {
    public:
        AdaptiveHoughGpuKernel(FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, SolutionsWriteAccessor solution);

        SYCL_EXTERNAL void operator()(Index2D idx) const;

    private:
        class AccumulatorSection
        {
        public:
            AccumulatorSection() = default;
            AccumulatorSection(double width, double height, double xBegin, double yBegin);

            double width;
            double height;
            double xBegin;
            double yBegin;
        };

        void fillAccumulatorSection(AccumulatorSection *sectionsStack, uint8_t &sectionsHeight, uint32_t *stubIndexes, uint32_t *stubCounts, uint8_t &divisionLevel, uint32_t *divisionLevelIterator) const;
        void fillHits(uint32_t *stubIndexes, uint32_t *stubCounts, uint8_t divisionLevel, const AccumulatorSection &section) const;
        void addSolution(double qOverPt_width, double phi_height, double qOverPtIndex, double phiIndex) const;

        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        SolutionsWriteAccessor solutions;
    };

} // namespace HelixSolver