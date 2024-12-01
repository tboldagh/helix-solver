#include "HelixSolverUsm/SingleRegionKernel.h"

SingleRegionKernel::SingleRegionKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result)
: splitter_(splitter)
, deviceNumPoints_(event->deviceNumPoints_)
, deviceXs_(event->deviceXs_)
, deviceYs_(event->deviceYs_)
, deviceZs_(event->deviceZs_)
, deviceLayers_(event->deviceLayers_)
, event_(event)
, result_(result) {}

void SingleRegionKernel::operator()(sycl::id<1> regionIdIdx) const
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

    float phis[MaxPointsInRegion];
    float rs[MaxPointsInRegion];

    AccumulatorRegion accumulatorRegions[MaxAccumulatorRegionStackSize];
    u_int8_t accumulatorRegionStackSize = 0;
    u_int32_t pointLists[MaxPointListsPointsNum];
    u_int32_t pointListsSizes[MaxPointListsNum];


    convertToPolarCoordinates(phis, rs, xs, ys, numPoints);

    const auto& wedge = splitter_->getSettings().wedges_[regionId - 1];
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
    accumulatorRegionStackSize = 1;

    // Fill point list for initial region
    for (u_int32_t i = 0; i < numPoints; ++i)
    {
        pointLists[i] = i;
    }
    pointListsSizes[0] = numPoints;

    while (accumulatorRegionStackSize > 0)
    {
        processNextAccumulatorRegion(accumulatorRegions, accumulatorRegionStackSize, pointLists, pointListsSizes);
    }

    if (rotateRegion)
    {
        rotateSolutions();
    }
}

void SingleRegionKernel::filterPointsInRegion(u_int16_t regionId, u_int32_t& numPoints, u_int32_t* indexes, float* xs, float* ys, float* zs, EventUsm::LayerNumber* layers) const
{
    // TODO: optimize by determining if the region is a pole or a wedge before iterating over all points
    for (uint32_t i = 0; i < *deviceNumPoints_; ++i)
    {
        if (splitter_->isPointInRegion(deviceXs_[i], deviceYs_[i], deviceZs_[i], regionId))
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

void SingleRegionKernel::processNextAccumulatorRegion(AccumulatorRegion* accumulatorRegions, u_int8_t& accumulatorRegionStackSize, u_int32_t* pointLists, u_int32_t* pointListsSizes) const
{
    // TODO: Implement
}

void SingleRegionKernel::rotateSolutions() const
{
    // Rotate solutions by pi
    // TODO: rotate solutions back
}
