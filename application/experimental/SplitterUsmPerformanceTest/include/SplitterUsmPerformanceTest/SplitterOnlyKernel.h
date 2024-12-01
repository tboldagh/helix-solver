#pragma once

#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "HelixSolverUsm/SingleRegionKernel.h"

#include <CL/sycl.hpp>


class SplitterOnlyKernel : public SingleRegionKernel
{
public:
    SplitterOnlyKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result);

    SYCL_EXTERNAL void operator()(sycl::id<1> regionIdIdx) const;
};