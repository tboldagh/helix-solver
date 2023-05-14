#pragma once

#include "AccumulatorSection.h"
#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/Debug.h"
#include "HelixSolver/EventBuffer.h"
#include "HelixSolver/AccumulatorSection.h"

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
        AdaptiveHoughGpuKernel(FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor z, SolutionsWriteAccessor solution);

        SYCL_EXTERNAL void operator()(Index2D idx) const;

    private:
        // data structure for region data
        struct RegionData {
            float one_over_RA[MAX_SP_PER_REGION];
            float phi[MAX_SP_PER_REGION];
            // TODO add z as well or index to the measurement in global memory
            uint16_t spInRegion = 0;
        };

        void fillAccumulatorSection(const RegionData& data, AccumulatorSection *sectionsStack, uint8_t &sectionsHeight) const;
        uint8_t countHits(const RegionData& data, AccumulatorSection &section) const;
        void addSolution(const RegionData& data, const AccumulatorSection& section) const;
        void fillPreciseSolution(const RegionData& data, const AccumulatorSection& section, SolutionCircle& s) const;

        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        FloatBufferReadAccessor zs;
        SolutionsWriteAccessor solutions;
    };

} // namespace HelixSolver