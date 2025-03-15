#include "HelixSolverUsm/SumRegionNumSolutionsKernel.h"


SumRegionNumSolutionsKernel::SumRegionNumSolutionsKernel(const ResultUsm* result)
: deviceNumSolutions_(result->kernelMemory_->numSolutions_)
, deviceRegionNumSolutions_(result->kernelMemory_->regionNumSolutions_) {}

SYCL_EXTERNAL void SumRegionNumSolutionsKernel::operator()() const
{
    u_int32_t numSolutions = 0;
    for (u_int16_t i = 0; i < ResultUsm::MaxRegions; ++i)
    {
        numSolutions += deviceRegionNumSolutions_[i];
        deviceRegionNumSolutions_[i] = numSolutions;
    }

    *deviceNumSolutions_ = numSolutions;
}