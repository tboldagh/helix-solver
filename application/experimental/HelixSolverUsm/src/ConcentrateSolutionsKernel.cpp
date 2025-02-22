#include "HelixSolverUsm/ConcentrateSolutionsKernel.h"

ConcentrateSolutionsKernel::ConcentrateSolutionsKernel(const ResultUsm* result)
: deviceRegionNumSolutions_(result->deviceNumRegionSolutions_)
, deviceSolutionHitCounts_(result->deviceSolutionHitCounts_)
, deviceSolutionRs_(result->deviceSolutionRs_)
, deviceSolutionPhis_(result->deviceSolutionPhis_) {}

void ConcentrateSolutionsKernel::operator()(sycl::id<1> index) const
{
    const u_int16_t idx = index[0];
    if (idx == 0)
    {
        // Solutions already in the right place
        return;
    }

    const u_int16_t regionNumSolutions =  deviceRegionNumSolutions_[idx] - deviceRegionNumSolutions_[idx - 1];
    const u_int32_t readRegionBegin = idx * ResultUsm::MaxSolutionsPerRegion;
    const u_int32_t writeRegionBegin = deviceRegionNumSolutions_[idx - 1];

    for (u_int32_t i = 0; i < regionNumSolutions; ++i)
    {
        const u_int32_t readIndex = readRegionBegin + i;
        const u_int32_t writeIndex = writeRegionBegin + i;
        deviceSolutionHitCounts_[writeIndex] = deviceSolutionHitCounts_[readIndex];
        deviceSolutionRs_[writeIndex] = deviceSolutionRs_[readIndex];
        deviceSolutionPhis_[writeIndex] = deviceSolutionPhis_[readIndex];
    }
}