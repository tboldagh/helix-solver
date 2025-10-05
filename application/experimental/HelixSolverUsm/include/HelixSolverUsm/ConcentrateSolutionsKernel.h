#pragma once

#include "EventUsm/ResultUsm.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest_prod.h>


class ConcentrateSolutionsKernel
{
public:
    ConcentrateSolutionsKernel(const ResultUsm* result);

    SYCL_EXTERNAL void operator()(sycl::id<1> index) const;

private:
    u_int32_t* deviceRegionNumSolutions_;
    uint8_t* deviceSolutionHitCounts_;
    float* deviceSolutionRs_;
    float* deviceSolutionPhis_;
};