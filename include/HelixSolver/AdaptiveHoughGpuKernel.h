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
using Index2D = sycl::id<2>;
#else
#include <vector>
#include <array>
using FloatBufferReadAccessor = const FloatBuffer &;
using SolutionsWriteAccessor = std::vector<HelixSolver::SolutionCircle> &;
using Index2D = std::array<int, 2>;
using OptionsBuffer = const HelixSolver::Options&;
#define SYCL_EXTERNAL
#endif

namespace HelixSolver
{
    class AdaptiveHoughGpuKernel
    {
    public:
        AdaptiveHoughGpuKernel(OptionsBuffer o, FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor zs, SolutionsWriteAccessor solution);

        SYCL_EXTERNAL void operator()(Index2D idx) const;

    private:
        void fillAccumulatorSection(AccumulatorSection *sectionsStack, uint8_t &sectionsHeight, std::vector<float> rs_wedge, std::vector<float> phis_wedge, std::vector<float> zs_wedge, float wedge_eta_center) const;
        uint8_t countHits(AccumulatorSection &section, std::vector<float> rs_wedge, std::vector<float> phis_wedge, std::vector<float> zs_wedge) const;
        uint8_t countHits_checkOrder(AccumulatorSection &section, std::vector<float> rs_wedge, std::vector<float> phis_wedge, std::vector<float> zs_wedge) const;
        void addSolution(const AccumulatorSection& section, float wedge_eta_center) const;
        void fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const;
        bool lineInsideAccumulator(const float radius_inverse, const float phi) const;
        float yLineAtBegin_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const;
        float yLineAtEnd_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const;
        bool isIntersectionWithinCell(const AccumulatorSection& section) const;
        bool isSolutionWithinCell(const AccumulatorSection& section) const;

        OptionsBuffer& opt;
        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        FloatBufferReadAccessor zs;
        SolutionsWriteAccessor solutions;
    };

} // namespace HelixSolver