#include "SplitterUsmPerformanceTest/SplitterOnlyKernel.h"

SplitterOnlyKernel::SplitterOnlyKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result, const SingleRegionKernelMemory* memory)
: SingleRegionKernel(splitter, event, result, memory) {}

void SplitterOnlyKernel::operator()(sycl::id<1> regionIdIdx) const
{
    Splitter splitter{*deviceSplitterSettings_};

    const u_int16_t regionId = regionIdIdx[0];
    if (regionId == 0)    // Invalid region, occurs because we cannot start parallel_for with 1
    {
        return;
    }

    if (regionId > splitter.getNumRegions() - 2)   // Pole
    {
        // Not sure if we need to care about pole regions, maybe implement later, skip for now
        return;
    }

    u_int32_t* indexes = kernelIndexes_;
    float* xs = kernelXs_;
    float* ys = kernelYs_;
    float* zs = kernelZs_;
    EventUsm::LayerNumber* layers = kernelLayers_;


    u_int32_t numPoints = 0;
    filterPointsInRegion(splitter, regionId, numPoints, indexes, xs, ys, zs, layers);

    for (uint32_t i = 0; i < numPoints; ++i)
    {
        deviceSolutionRs_[regionIdIdx] += indexes[i] + xs[i] + ys[i] + zs[i] + layers[i];
    }
    *deviceNumSolutions_ = splitter.getNumRegions();
}