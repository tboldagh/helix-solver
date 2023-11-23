#pragma once

#include "AccumulatorSection.h"
#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/Debug.h"
#include "HelixSolver/EventBuffer.h"
#include "HelixSolver/AccumulatorSection.h"
#include "HelixSolver/Options.h"
#include "HelixSolver/ZPhiPartitioning.h"

#ifdef USE_SYCL
#include <CL/sycl.hpp>
using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
using SolutionsWriteAccessor = sycl::accessor<HelixSolver::SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device>;
using OptionsAccessor = sycl::accessor<HelixSolver::Options, 1, sycl::access::mode::read, sycl::access::target::device>;
using Index2D = sycl::id<2>;
#else
#include <vector>
#include <array>
using FloatBufferReadAccessor = const FloatBuffer &;
using SolutionsWriteAccessor = std::vector<HelixSolver::SolutionCircle> &;
using Index2D = std::array<int, 2>;
using OptionsAccessor = const OptionsBuffer &;
#define SYCL_EXTERNAL
#endif

namespace HelixSolver
{
    class AdaptiveHoughGpuKernel
    {
    public:
        AdaptiveHoughGpuKernel(OptionsAccessor o, FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor z, SolutionsWriteAccessor solution);

        SYCL_EXTERNAL void operator()(Index2D idx) const;

    private:
        void fillAccumulatorSection(AccumulatorSection *sectionsStack, uint32_t &sectionsHeight, float* rs_wedge, float* phis_wedge, float* zs_wedge, float wedge_eta_center, uint32_t wedge_spacepoints_count) const;
        uint16_t countHits(AccumulatorSection &section, float* rs_wedge, float* phis_wedge, float* zs_wedge, uint32_t wedge_spacepoints_count) const;
        uint16_t countHits_checkOrder(AccumulatorSection &section, const float* rs_wedge, const float* phis_wedge, const float* zs_wedge, const uint32_t wedge_spacepoints_count) const;
        void addSolution(const AccumulatorSection& section, float wedge_eta_center) const;
        void fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const;
        bool lineInsideAccumulator(const float radius_inverse, const float phi) const;
        bool isIntersectionWithinCell(const AccumulatorSection& section) const;
        bool isPeakWithinCell(const AccumulatorSection& section) const;
        bool lineInsideCell(const AccumulatorSection section, const float radius_inverse, const float phi) const;
        float* sortArrays(const float* distance_array, const float* id_array, const uint32_t counter) const;

        OptionsAccessor opts;
        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        FloatBufferReadAccessor zs;
        SolutionsWriteAccessor solutions;
    };

} // namespace HelixSolver