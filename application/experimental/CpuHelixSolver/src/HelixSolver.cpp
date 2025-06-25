#include "CpuHelixSolver/HelixSolver.h"
#include "Logger/Logger.h"

#include <chrono>
#include <sstream>

HelixSolver::HelixSolver(const Splitter& splitter)
    : splitter_(splitter)
{
    const u_int16_t numWedges = splitter_.getNumRegions() - 2;
    regionSolverData_.reserve(numWedges);
    for (u_int16_t i = 0; i < numWedges; ++i)
    {
        regionSolverData_.emplace_back();
    }
}

void HelixSolver::solve(Task& task)
{
    const u_int16_t numWedges = splitter_.getNumRegions() - 2;
    std::vector<u_int32_t*> regionIndexes;
    regionIndexes.reserve(numWedges);
    std::vector<u_int32_t*> regionNumPoints;
    regionNumPoints.reserve(numWedges);
    for (u_int16_t i = 0; i < numWedges; ++i)
    {
        regionIndexes.emplace_back(regionSolverData_[i].indexes_);
        regionNumPoints.emplace_back(&regionSolverData_[i].numPoints_);
    }
    splitter_.splitIntoRegions(task.getEvent().xs_, task.getEvent().ys_, task.getEvent().zs_, task.getEvent().numPoints_, regionIndexes, regionNumPoints, numWedges);

    // Reset number of solutions
    task.getResult().numSolutions_ = 0;

    // Not sure if we need to care about pole regions, maybe implement later, skip for now
    for (u_int16_t regionIndex = 0; regionIndex < numWedges; ++regionIndex)
    {
        regionSolverData_[regionIndex].regionId_ = regionIndex + 1;
        solveRegion(task, regionSolverData_[regionIndex]);
    }
}

void HelixSolver::solveRegion(Task& task, RegionSolverData& regionSolverData)
{
    const Event& event = task.getEvent();
    Result& result = task.getResult();
    const u_int16_t regionId = regionSolverData.regionId_;

    regionSolverData.regionSolutionHitsThreshold_ = splitter_.getSettings().wedges_[regionId - 1].solutionHitsThreshold_;
    regionSolverData.regionSolutionHitsThreshold_ = regionSolverData.regionSolutionHitsThreshold_ > 0 ? regionSolverData.regionSolutionHitsThreshold_ : 8;
    regionSolverData.regionLinesCrossingsThreshold_ = splitter_.getSettings().wedges_[regionId - 1].linesCrossingsThreshold_;
    regionSolverData.regionLinesCrossingsThreshold_ = regionSolverData.regionLinesCrossingsThreshold_ > 0 ? regionSolverData.regionLinesCrossingsThreshold_ : 3;

    convertToPolarCoordinates(event, regionSolverData);

    assignLayers(event, regionSolverData);

    const auto& wedge = splitter_.getSettings().wedges_[regionId - 1];
    float regionPhi0Min = wrapMinusPiToPi(wedge.zAngleMin_) - SpaceMaxPhiPhi0AbsDiff;
    float regionPhi0Max = wrapMinusPiToPi(wedge.zAngleMax_) + SpaceMaxPhiPhi0AbsDiff;

    // Due to atan2 discontinuity close to 0 in the negative x plane, we need to rotate the region and points
    // to make sure helixes crossing y=0 are correctly detected. Then we have to rotate the results back.
    const bool rotateRegion = regionPhi0Min > regionPhi0Max;  // Region crosses y=0 in negative x
    u_int32_t regionSolutionsBegin = result.numSolutions_;
    if (rotateRegion)
    {
        rotateRegionAndPoints(regionPhi0Min, regionPhi0Max, regionSolverData);
    }

    // Add initial region
    regionSolverData.accumulatorRegions_[0] = AccumulatorRegion(SpaceMinQOverPt, SpaceMaxQOverPt, regionPhi0Min, regionPhi0Max);
    regionSolverData.accumulatorRegions_[0].pointListBegin_ = 0;
    regionSolverData.accumulatorRegionStackSize_ = 1;

    // Fill point list for initial region
    const AccumulatorRegion& initialRegion = regionSolverData.accumulatorRegions_[0];
    u_int32_t pointListsEnd = 0;
    for (u_int32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        if (regionHit(initialRegion, regionSolverData.rs_[i], regionSolverData.phis_[i]))
        {
            regionSolverData.pointLists_[pointListsEnd++] = i;
        }
    }
    regionSolverData.accumulatorRegions_[0].pointListEnd_ = pointListsEnd;

    while (regionSolverData.accumulatorRegionStackSize_ > 0)
    {
        processNextAccumulatorRegion(result, regionSolverData);
    }

    if (rotateRegion)
    {
        // Reverse rotation when we rotate region and points
        rotateSolutions(result, regionSolutionsBegin);
    }
}

void HelixSolver::filterPointsInWedge(u_int16_t regionId, const Event& event, RegionSolverData& regionSolverData)
{
    float* xs = event.xs_;
    float* ys = event.ys_;
    float* zs = event.zs_;
    u_int32_t* indexes = regionSolverData.indexes_;
    for (u_int32_t i = 0; i < event.numPoints_; ++i)
    {
        if (splitter_.isPointInRegion(xs[i], ys[i], zs[i], regionId))
        {
            indexes[regionSolverData.numPoints_++] = i;
        }
    }
}

void HelixSolver::convertToPolarCoordinates(const Event& event, RegionSolverData& regionSolverData)
{
    float* xs = event.xs_;
    float* ys = event.ys_;
    float* rs = regionSolverData.rs_;
    float* phis = regionSolverData.phis_;
    for (u_int32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        const u_int32_t index = regionSolverData.indexes_[i];
        const float x = xs[index];
        const float y = ys[index];
        rs[i] = std::sqrt(x * x + y * y);
        phis[i] = std::atan2(y, x);
    }
}

void HelixSolver::assignLayers(const Event& event, RegionSolverData& regionSolverData)
{
    float* rs = regionSolverData.rs_;
    float* zs = event.zs_;
    u_int8_t* layers = regionSolverData.layers_;
    for (u_int32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        const float r = rs[i];
        const float absZ = std::abs(zs[i]);

        if (r < 215)
        {
            // Group A or B
            if (absZ < 550)
            {
                // Group A
                layers[i] = r < 45 ? 0 :
                            r < 90 ? 1 :
                            r < 140 ? 2 : 3;
            }
            else
            {
                // Group B
                layers[i] = absZ < 670 ? 4 :
                            absZ < 780 ? 5 :
                            absZ < 930 ? 6 :
                            absZ < 1050 ? 7 :
                            absZ < 1220 ? 8 :
                            absZ < 1420 ? 9 : 10;
            }
        }
        else
        {
            // Group C or D or E
            if (r > 740)
            {
                // Group D
                layers[i] = absZ < 870 ? 11 : 12;
            }
            else if (absZ < 1200)
            {
                // Group C
                layers[i] = r < 310 ? 13 :
                            r < 430 ? 14 :
                            r < 580 ? 15 : 16;
            }
            else
            {
                // Group E
                layers[i] = absZ < 1440 ? 17 :
                            absZ < 1700 ? 18 :
                            absZ < 2030 ? 19 :
                            absZ < 2370 ? 20 :
                            absZ < 2730 ? 21 : 22;
            }
        }
    }
}

void HelixSolver::rotateRegionAndPoints(float& regionPhi0Min, float& regionPhi0Max, RegionSolverData& regionSolverData)
{
    // Rotate region by -pi
    regionPhi0Min -= M_PI;
    regionPhi0Max += M_PI;
    float* phis = regionSolverData.phis_;
    for (uint32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        phis[i] = wrapMinusPiToPi(phis[i] - M_PI);
    }
}

void HelixSolver::processNextAccumulatorRegion(Result& result, RegionSolverData& regionSolverData)
{
    uint8_t& accumulatorRegionStackSize = regionSolverData.accumulatorRegionStackSize_;
    AccumulatorRegion* accumulatorRegions = regionSolverData.accumulatorRegions_;

    // Pop region from stack
    accumulatorRegionStackSize--;
    const AccumulatorRegion region = accumulatorRegions[accumulatorRegionStackSize];
    if (!enoughHitsAndLinesCrossing(region, regionSolverData))
    {
        return;
    }

    if (region.qOverPtDivisionLevel_ < RegionSolverData::QOverPtMaxDivisionLevel && region.phi0DivisionLevel_ < RegionSolverData::Phi0MaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMinPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = region.pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = region.pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;
    
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMinPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMaxPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 2].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 2].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMaxPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 3].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 3].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;
    }
    else if (region.qOverPtDivisionLevel_ < RegionSolverData::QOverPtMaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMin();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = region.pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = region.pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMax();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;
    }
    else if (region.phi0DivisionLevel_ < RegionSolverData::Phi0MaxDivisionLevel)
    {
        accumulatorRegions[accumulatorRegionStackSize] = region.subregionPhi0Min();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = region.pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = region.pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;
    }
    else
    {
        // Max division level reached, add solution
        if (enoughLayerHits(regionSolverData))
        {
            addSolution(result, region);
        }
    }
}

bool HelixSolver::enoughHitsAndLinesCrossing(const AccumulatorRegion& region, const RegionSolverData& regionSolverData)
{
    const u_int32_t numHits = region.pointListEnd_ - region.pointListBegin_;
    if (numHits < regionSolverData.regionSolutionHitsThreshold_)
    {
        // Too few points to form a helix, no solution in this region
        return false;
    }

    if (numHits > 8 * regionSolverData.regionSolutionHitsThreshold_)
    {
        // Too many points in region, assume enough crossings
        return true;
    }

    const float qOverPtMin = region.qOverPtMin_;
    const float qOverPtMax = region.qOverPtMax_;
    const u_int8_t linesCrossingsThreshold = regionSolverData.regionLinesCrossingsThreshold_;

    u_int8_t linesWithEnoughCrossings = 0;
    for (u_int32_t i = region.pointListBegin_; i < region.pointListEnd_; ++i)
    {
        const float r = regionSolverData.rs_[regionSolverData.pointLists_[i]];
        const float phi = regionSolverData.phis_[regionSolverData.pointLists_[i]];
        const u_int8_t layer = regionSolverData.layers_[regionSolverData.pointLists_[i]];
        const float phi0Left = - 0.5f * BMagnitude * r * qOverPtMin + phi;
        const float phi0Right = - 0.5f * BMagnitude * r * qOverPtMax + phi;

        u_int8_t crossings = 0;
        for (u_int32_t j = region.pointListBegin_; j < region.pointListEnd_; ++j)
        {
            if (i == j)
            {
                continue;
            }

            const u_int8_t layerOther = regionSolverData.layers_[regionSolverData.pointLists_[j]];
            if (layer == layerOther)
            {
                continue;
            }

            const float rOther = regionSolverData.rs_[regionSolverData.pointLists_[j]];
            const float phiOther = regionSolverData.phis_[regionSolverData.pointLists_[j]];
            const float phi0LeftOther = - 0.5f * BMagnitude * rOther * qOverPtMin + phiOther;
            const float phi0RightOther = - 0.5f * BMagnitude * rOther * qOverPtMax + phiOther;

            // Check if lines cross within region
            if ((phi0Left > phi0LeftOther && phi0Right < phi0RightOther) || (phi0Left < phi0LeftOther && phi0Right > phi0RightOther))
            {
                crossings++;
                if (crossings >= linesCrossingsThreshold)
                {
                    break;
                }
            }
        }

        if (crossings >= linesCrossingsThreshold)
        {
            linesWithEnoughCrossings++;
            if (linesWithEnoughCrossings >= linesCrossingsThreshold)
            {
                return true;
            }
        }
    }

    return false;
}

void HelixSolver::fillNewPointList(AccumulatorRegion& region, const AccumulatorRegion& sourceRegion, RegionSolverData& regionSolverData)
{
    for (u_int32_t i = sourceRegion.pointListBegin_; i < sourceRegion.pointListEnd_; ++i)
    {
        const u_int32_t index = regionSolverData.pointLists_[i];
        if (regionHit(region, regionSolverData.rs_[index], regionSolverData.phis_[index]))
        {
            regionSolverData.pointLists_[region.pointListEnd_++] = index;
        }
    }
}

bool HelixSolver::regionHit(const AccumulatorRegion& region, const float r, const float phi)
{
    const float qOverPtMin = region.qOverPtMin_;
    const float qOverPtMax = region.qOverPtMax_;
    const float phi0Min = region.phi0Min_;
    const float phi0Max = region.phi0Max_;

    // See thesis p. 56
    const float phi0Left = - 0.5f * BMagnitude * r * qOverPtMin + phi;
    const float phi0Right = - 0.5f * BMagnitude * r * qOverPtMax + phi;
    return phi0Left >= phi0Min && phi0Right <= phi0Max;
}

bool HelixSolver::enoughLayerHits(const RegionSolverData& regionSolverData)
{
    u_int32_t layersHit = 0;
    for (u_int32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        layersHit |= 1 << regionSolverData.layers_[regionSolverData.pointLists_[i]];
    }

    uint8_t hitCount = 0;
    for (u_int32_t i = 0; i < 23; ++i)
    {
        if (layersHit & (1 << i))
        {
            hitCount++;
        }
    }

    return hitCount >= RegionSolverData::LayerHitThreshold;
}

void HelixSolver::addSolution(Result& result, const AccumulatorRegion& region)
{
    const float qOverPt = 0.5f * (region.qOverPtMin_ + region.qOverPtMax_);
    const float phi0 = 0.5f * (region.phi0Min_ + region.phi0Max_);

    // See thesis p. 24
    const float r = 1 / (qOverPt * BMagnitude);
    const float phi = wrapMinusPiToPi(phi0 + 0.5f * M_PI);

    const u_int32_t index = result.numSolutions_++;
    const u_int32_t numHits = region.pointListEnd_ - region.pointListBegin_;
    result.solutionHitCounts_[index] = numHits > 255 ? 255 : numHits;
    result.solutionRs_[index] = r;
    result.solutionPhis_[index] = phi;
}

void HelixSolver::rotateSolutions(Result& result, const u_int32_t regionSolutionsBegin)
{
    // Rotate solutions by pi
    const u_int32_t numSolutions = result.numSolutions_;
    for (u_int32_t i = regionSolutionsBegin; i < numSolutions; ++i)
    {
        result.solutionPhis_[i] = wrapMinusPiToPi(result.solutionPhis_[i] + M_PI);
    }
}
