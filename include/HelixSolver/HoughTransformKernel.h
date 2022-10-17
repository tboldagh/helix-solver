#pragma once

#include <CL/sycl.hpp>

#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"

namespace HelixSolver
{
    class HoughTransformKernel
    {
    public:
        HoughTransformKernel(sycl::handler& h,
                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                            sycl::buffer<float, 1>& rBuffer,
                            sycl::buffer<float, 1>& phiBuffer);

        SYCL_EXTERNAL void operator()() const;

    private:
        void fillAccumulator(float* rs, float* phis, uint8_t* accumulator) const;

        void transferSolutionToHostDevice(uint8_t* accumulator) const;

        static uint32_t mapToBeanIndex(float y);

        void transferDataToBoardMemory(float* rs, float* phis) const;

        // TODO: remove hungarian notation
        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> m_mapAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> m_rAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> m_phiAccessor;
    };
} // namespace HelixSolver
