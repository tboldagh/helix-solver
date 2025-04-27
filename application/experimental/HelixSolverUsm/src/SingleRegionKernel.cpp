#include "HelixSolverUsm/SingleRegionKernel.h"


SingleRegionKernel::SingleRegionKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result, const SingleRegionKernelMemory* memory)
: deviceSplitterSettings_(splitter->splitterSettingsKernelMemory_->settings_)
, deviceNumPoints_(event->kernelMemory_->numPoints_)
, deviceXs_(event->kernelMemory_->xs_)
, deviceYs_(event->kernelMemory_->ys_)
, deviceZs_(event->kernelMemory_->zs_)
, deviceLayers_(event->kernelMemory_->layers_)
, deviceNumSolutions_(result->kernelMemory_->numSolutions_)
, deviceRegionNumSolutions_(result->kernelMemory_->regionNumSolutions_)
, deviceSolutionHitCounts_(result->kernelMemory_->solutionHitCounts_)
, deviceSolutionRs_(result->kernelMemory_->solutionRs_)
, deviceSolutionPhis_(result->kernelMemory_->solutionPhis_)
, event_(event)
, result_(result)
, kernelIndexes_(memory->indexes_)
, kernelXs_(memory->xs_)
, kernelYs_(memory->ys_)
, kernelZs_(memory->zs_)
, kernelLayers_(memory->layers_)
, kernelPointLists_(memory->pointLists_)
, kernelRs_(memory->rs_)
, kernelPhis_(memory->phis_) {}

void SingleRegionKernel::operator()(sycl::id<1> regionIdIdx) const
{
    Splitter splitter{*deviceSplitterSettings_};

    const u_int16_t regionId = regionIdIdx[0] + 1;   // 0 is reserved for invalid region

    if (regionId > splitter.getNumRegions() - 2)   // Pole
    {
        // Not sure if we need to care about pole regions, maybe implement later, skip for now
        return;
    }

    // if (regionId > 3)
    // {
    //     return;
    // }

    u_int32_t numPoints = 0;
    u_int32_t* indexes = kernelIndexes_ + (regionId - 1) * MaxPointsInRegion;
    float* xs = kernelXs_ + (regionId - 1) * MaxPointsInRegion;
    float* ys = kernelYs_ + (regionId - 1) * MaxPointsInRegion;
    float* zs = kernelZs_ + (regionId - 1) * MaxPointsInRegion;
    EventUsm::LayerNumber* layers = kernelLayers_ + (regionId - 1) * MaxPointsInRegion;

    filterPointsInRegion(splitter, regionId, numPoints, indexes, xs, ys, zs, layers);

    float* rs = kernelRs_ + (regionId - 1) * MaxPointsInRegion;
    float* phis = kernelPhis_ + (regionId - 1) * MaxPointsInRegion;

    AccumulatorRegion accumulatorRegions[MaxAccumulatorRegionStackSize];
    u_int8_t accumulatorRegionStackSize = 0;
    u_int32_t* pointLists = kernelPointLists_ + (regionId - 1) * MaxPointListsPointsNum;


    convertToPolarCoordinates(phis, rs, xs, ys, numPoints);

    const auto& wedge = splitter.getSettings().wedges_[regionId - 1];
    float regionPhi0Min = wrapMinusPiToPi(wedge.zAngleMin_) - SpaceMaxPhiPhi0AbsDiff;
    float regionPhi0Max = wrapMinusPiToPi(wedge.zAngleMax_) + SpaceMaxPhiPhi0AbsDiff;

    // Due to atan2 discontinuity close to 0 in the negative x plane, we need to rotate the region and points
    // to make sure helixes crossing y=0 are correctly detected. Then we have to rotate the results back.
    const bool rotateRegion = regionPhi0Min > regionPhi0Max;  // Region crosses y=0 in negative x
    if (rotateRegion)
    {
        rotateRegionAndPoints(regionPhi0Min, regionPhi0Max, phis, numPoints);
    }

    // Add initial region
    accumulatorRegions[0] = AccumulatorRegion(SpaceMinQOverPt, SpaceMaxQOverPt, regionPhi0Min, regionPhi0Max);
    accumulatorRegions[0].pointListBegin = 0;
    accumulatorRegionStackSize = 1;

    // Fill point list for initial region
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        pointLists[i] = i;
    }
    accumulatorRegions[0].pointListEnd = numPoints;

    while (accumulatorRegionStackSize > 0)
    {
        processNextAccumulatorRegion(regionId, accumulatorRegions, accumulatorRegionStackSize, pointLists, indexes, rs, phis, deviceRegionNumSolutions_, deviceSolutionHitCounts_, deviceSolutionRs_, deviceSolutionPhis_);
    }

    if (rotateRegion)
    {
        rotateSolutions(regionId);
    }
}

std::unique_ptr<DeviceResourceGroup> SingleRegionKernel::createResourceGroup(sycl::queue& queue)
{
    auto* eventMemory = new EventUsm::EventKernelMemory{queue};
    auto* resultMemory = new ResultUsm::ResultKernelMemory{queue};
    auto* kernelMemory = new SingleRegionKernelMemory{queue};
    auto* splitterSettingsMemory = new Splitter::SplitterSettingsKernelMemory{queue};
    eventMemory->allocate();
    resultMemory->allocate();
    kernelMemory->allocate();
    splitterSettingsMemory->allocate();
    return std::make_unique<DeviceResourceGroup>(DeviceResourceGroup{
        {DeviceResourceType::EventKernelMemory, eventMemory},
        {DeviceResourceType::ResultKernelMemory, resultMemory},
        {DeviceResourceType::KernelMemory, kernelMemory},
        {DeviceResourceType::SplitterSettingsKernelMemory, splitterSettingsMemory}
    });
}

void SingleRegionKernel::filterPointsInRegion(const Splitter& splitter, u_int16_t regionId, u_int32_t& numPoints, u_int32_t* indexes, float* xs, float* ys, float* zs, EventUsm::LayerNumber* layers) const
{
    // TODO: optimize by determining if the region is a pole or a wedge before iterating over all points
    for (uint32_t i = 0; i < *deviceNumPoints_; ++i)
    {
        if (splitter.isPointInRegion(deviceXs_[i], deviceYs_[i], deviceZs_[i], regionId))
        {
            indexes[numPoints] = i;
            xs[numPoints] = deviceXs_[i];
            ys[numPoints] = deviceYs_[i];
            zs[numPoints] = deviceZs_[i];
            layers[numPoints] = deviceLayers_[i];
            numPoints++;
        }
    }
}

void SingleRegionKernel::convertToPolarCoordinates(float* phis, float* rs, const float* xs, const float* ys, u_int32_t numPoints) const
{
    for (uint32_t i = 0; i < numPoints; ++i)
    {
        phis[i] = std::atan2(ys[i], xs[i]);
        rs[i] = std::sqrt(xs[i] * xs[i] + ys[i] * ys[i]);
    }
}

void SingleRegionKernel::rotateRegionAndPoints(float& regionPhi0Min, float& regionPhi0Max, float* phis, u_int32_t numPoints) const
{
    // Rotate region by -pi
    regionPhi0Min -= M_PI;
    regionPhi0Max += M_PI;
    for (uint32_t i = 0; i < numPoints; ++i)
    {
        phis[i] = wrapMinusPiToPi(phis[i] - M_PI);
    }
}

void SingleRegionKernel::processNextAccumulatorRegion(u_int16_t regionId, AccumulatorRegion* accumulatorRegions, u_int8_t& accumulatorRegionStackSize, u_int32_t* pointLists, const u_int32_t* indexes, const float* rs, const float* phis, u_int32_t* regionNumSolutions, u_int8_t* solutionHitCounts, float* solutionRs, float* solutionPhis)
{
    // Pop region from stack
    accumulatorRegionStackSize--;
    const AccumulatorRegion region = accumulatorRegions[accumulatorRegionStackSize];

    const u_int32_t numHits = region.pointListEnd - region.pointListBegin;
    if (numHits < SolutionHitsThreshold)
    {
        // Too few points to form a helix, no solution in this region
        return;
    }

    if (region.qOverPtDivisionLevel < QOverPtMaxDivisionLevel && region.phi0DivisionLevel < Phi0MaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMinPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = region.pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = region.pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;
    
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMinPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMaxPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMaxPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;
    }
    else if (region.qOverPtDivisionLevel < QOverPtMaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMin();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = region.pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = region.pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMax();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;
    }
    else if (region.phi0DivisionLevel < Phi0MaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = region.pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = region.pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, pointLists, rs, phis);
        accumulatorRegionStackSize++;
    }
    else
    {
        // Max division level reached, add solution

        addSolution(regionId, region, regionNumSolutions, solutionHitCounts, solutionRs, solutionPhis);
    }
}

void SingleRegionKernel::fillNewPointList(AccumulatorRegion& region, const AccumulatorRegion& sourceRegion, u_int32_t* pointLists, const float* rs, const float* phis)
{
    for (u_int32_t i = sourceRegion.pointListBegin; i < sourceRegion.pointListEnd; ++i)
    {
        const u_int32_t index = pointLists[i];
        if (regionHit(region.qOverPtMin, region.qOverPtMax, region.phi0Min, region.phi0Max, rs[index], phis[index]))
        {
            pointLists[region.pointListEnd++] = index;
        }
    }
}

bool SingleRegionKernel::regionHit(float qOverPtMin, float qOverPtMax, float phi0Min, float phi0Max, float r, float phi)
{
    // See thesis p. 56
    const float phi0Left = - 0.5f * BMagnitude * r * qOverPtMin + phi;
    const float phi0Right = - 0.5f * BMagnitude * r * qOverPtMax + phi;
    return phi0Left >= phi0Min && phi0Right <= phi0Max;
}

void SingleRegionKernel::addSolution(u_int16_t regionId, const AccumulatorRegion& region, uint32_t* regionNumSolutions, u_int8_t* solutionHitCounts, float* solutionRs, float* solutionPhis)
{
    const float qOverPt = 0.5f * (region.qOverPtMin + region.qOverPtMax);
    const float phi0 = 0.5f * (region.phi0Min + region.phi0Max);

    // See thesis p. 24
    const float r = 1 / (qOverPt * BMagnitude);
    const float phi = wrapMinusPiToPi(phi0 + 0.5f * M_PI);

    const u_int32_t index = regionId - 1;
    const u_int32_t solutionIndex = index * ResultUsm::MaxSolutionsPerRegion + regionNumSolutions[index];
    const u_int32_t numHits = region.pointListEnd - region.pointListBegin;
    regionNumSolutions[index]++;
    solutionHitCounts[solutionIndex] = numHits > 255 ? 255 : numHits;
    solutionRs[solutionIndex] = r;
    solutionPhis[solutionIndex] = phi;
}

void SingleRegionKernel::rotateSolutions(u_int16_t regionId) const
{
    // Rotate solutions by pi
    const u_int32_t numSolutions = deviceRegionNumSolutions_[regionId - 1];
    for (u_int32_t i = 0; i < numSolutions; ++i)
    {
        deviceSolutionPhis_[i] = wrapMinusPiToPi(deviceSolutionPhis_[i] + M_PI);
    }
}
