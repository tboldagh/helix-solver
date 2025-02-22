#pragma once

#include "EventUsm/ResultUsm.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest_prod.h>


class SumRegionNumSolutionsKernel
{
public:
    SumRegionNumSolutionsKernel(const ResultUsm* result);

    SYCL_EXTERNAL void operator()() const;

private:
    u_int32_t* deviceNumSolutions_;
    u_int32_t* deviceRegionNumSolutions_;
};