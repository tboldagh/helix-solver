#include "SplitterUsmPerformanceTest/SplitterOnlyKernel.h"

SplitterOnlyKernel::SplitterOnlyKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result)
: SingleRegionKernel(splitter, event, result) {}

void SplitterOnlyKernel::operator()(sycl::id<1> regionIdIdx) const
{
    const u_int16_t regionId = regionIdIdx[0];
    if (regionId == 0)    // Invalid region, occurs because we cannot start parallel_for with 1
    {
        return;
    }

    if (regionId > splitter_->getNumRegions() - 2)   // Pole
    {
        // Not sure if we need to care about pole regions, maybe implement later, skip for now
        return;
    }

    u_int32_t numPoints = 0;
    u_int32_t indexes[MaxPointsInRegion];   // Just in case we need to keep info about which points form a helix
    float xs[MaxPointsInRegion];
    float ys[MaxPointsInRegion];
    float zs[MaxPointsInRegion];
    EventUsm::LayerNumber layers[MaxPointsInRegion];

    filterPointsInRegion(regionId, numPoints, indexes, xs, ys, zs, layers);

    for (uint32_t i = 0; i < numPoints; ++i)
    {
        deviceSomeSolutionParameters_[regionIdIdx] += indexes[i] + xs[i] + ys[i] + zs[i] + layers[i];
    }
}