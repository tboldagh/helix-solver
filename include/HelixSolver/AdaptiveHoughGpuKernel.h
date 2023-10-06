#pragma once

#include "AccumulatorSection.h"
#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"
#include "HelixSolver/Debug.h"
#include "HelixSolver/EventBuffer.h"
#include "HelixSolver/AccumulatorSection.h"
#include "HelixSolver/Options.h"

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
        void fillAccumulatorSection(AccumulatorSection *sectionsStack, uint8_t &sectionsHeight) const;
        uint8_t countHits(AccumulatorSection &section) const;
        uint8_t countHits_checkOrder(AccumulatorSection &section) const;
        void addSolution(const AccumulatorSection& section) const;
        void fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const;
        bool lineInsideAccumulator(const float radius_inverse, const float phi) const;
        float yLineAtBegin_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const;
        float yLineAtEnd_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const;
        bool isIntersectionWithinCell(const AccumulatorSection& section) const;
        bool isSolutionWithinCell(const AccumulatorSection& section) const;

        OptionsAccessor opts;
        FloatBufferReadAccessor rs;
        FloatBufferReadAccessor phis;
        SolutionsWriteAccessor solutions;
    };

} // namespace HelixSolver