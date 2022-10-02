#pragma once

#include <CL/sycl.hpp>

#include "HelixSolver/SolutionCircle.h"
#include "HelixSolver/Constants.h"

namespace HelixSolver
{
    class HoughTransformKernel
    {
    public:
        HoughTransformKernel() = default;

        HoughTransformKernel(sycl::handler &h,
                            sycl::buffer<SolutionCircle, 1> &mapBuffer,
                            sycl::buffer<float, 1> &rBuffer,
                            sycl::buffer<float, 1> &phiBuffer,
                            sycl::buffer<uint8_t, 1> &layersBuffer,
                            sycl::buffer<float, 1> &xLinspaceBuf,
                            sycl::buffer<float, 1> &yLinspaceBuf);

        SYCL_EXTERNAL void operator()() const;

    private:
        void fillBoardAccumulator(float *X,
                                float *R,
                                float *PHI,
                                uint8_t *LAYER,
                                bool ACCUMULATOR[][ACC_SIZE]) const;

        void transferSolutionToHostDevice(bool ACCUMULATOR[][ACC_SIZE]) const;

        static float calculateAngle(float r, float x, float phi);

        static uint32_t mapToBeanIndex(float y);

        void transferDataToBoardMemory(float* x,
                                    float* y,
                                    float* r,
                                    float* phi,
                                    uint8_t* layer) const;

        // TODO: remove hungarian notation
        sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::global_buffer> m_mapAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::global_buffer> m_rAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::global_buffer> m_phiAccessor;
        sycl::accessor<uint8_t, 1, sycl::access::mode::read, sycl::access::target::global_buffer> m_layersAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::global_buffer> m_xLinspaceAccessor;
        sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::global_buffer> m_yLinspaceAccessor;
    };
} // namespace HelixSolver
