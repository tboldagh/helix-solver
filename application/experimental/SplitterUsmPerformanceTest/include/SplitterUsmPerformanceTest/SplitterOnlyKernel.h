#pragma once

#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "HelixSolverUsm/SingleRegionKernel.h"

#include <sycl/sycl.hpp>


class SplitterOnlyKernel : public SingleRegionKernel
{
public:
    SplitterOnlyKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result, const SingleRegionKernelMemory* memory);

    SYCL_EXTERNAL void operator()(sycl::id<1> regionIdIdx) const;
};