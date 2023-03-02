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
            AccumulatorSection(float width, float height, float xBegin, float yBegin);

            float width;
            float height;
            float xBegin;
            float yBegin;
        };

        void fillAccumulatorSection(AccumulatorSection *sectionsStack, uint8_t &sectionsHeight, uint32_t *stubIndexes, uint32_t *stubCounts) const;
        void fillHits(uint32_t *stubIndexes, uint32_t *stubCounts, uint8_t divisionLevel, const AccumulatorSection &section) const;
        void addSolution(uint32_t qOverPtIndex, uint32_t phiIndex) const;
        static inline float linspaceElement(float start, float end, float numPoints, float index);

        static constexpr uint8_t MAX_DIVISION_LEVEL = Q_OVER_PT_MAX_GRID_DIVISION_LEVEL > PHI_MAX_GRID_DIVISION_LEVEL ? Q_OVER_PT_MAX_GRID_DIVISION_LEVEL : PHI_MAX_GRID_DIVISION_LEVEL;

        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        SolutionsWriteAccessor solutions;
    };

    inline float AdaptiveHoughGpuKernel::linspaceElement(float start, float end, float numPoints, float index)
    {
        return start + (end - start) * index / (numPoints - 1);
    }

} // namespace HelixSolver