#include "CpuHelixSolver/HelixSolver.h"

HelixSolver::HelixSolver(const Splitter& splitter)
    : splitter_(splitter)
{
}

void HelixSolver::solve(Task& task)
{
    RegionSolverData regionSolverData;

    // Reset number of solutions
    task.getResult().numSolutions_ = 0;

    // 0 is reserved for invalid region
    for (u_int16_t regionId = 1; regionId < splitter_.getNumRegions() - 1; ++regionId)
    {
        regionSolverData.regionId_ = regionId;
        solveRegion(task, regionSolverData);
    }
}

void HelixSolver::solveRegion(Task& task, RegionSolverData& regionSolverData)
{
    const Event& event = task.getEvent();
    Result& result = task.getResult();

    const u_int16_t regionId = regionSolverData.regionId_;
    if (regionId > splitter_.getNumRegions() - 2)   // Pole
    {
        // Not sure if we need to care about pole regions, maybe implement later, skip for now
        return;
    }

    // Reset necessary fields in regionSolverData
    regionSolverData.numPoints_ = 0;

    filterPointsInWedge(regionId, event, regionSolverData);

    convertToPolarCoordinates(event, regionSolverData);

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
    for (u_int32_t i = 0; i < regionSolverData.numPoints_; ++i)
    {
        regionSolverData.pointLists_[i] = i;
    }
    regionSolverData.accumulatorRegions_[0].pointListEnd_ = regionSolverData.numPoints_;

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

    const u_int32_t numHits = region.pointListEnd_ - region.pointListBegin_;
    if (numHits < SolutionHitsThreshold)
    {
        // Too few points to form a helix, no solution in this region
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
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        fillNewPointList(accumulatorRegions[accumulatorRegionStackSize], region, regionSolverData);
        accumulatorRegionStackSize++;

        accumulatorRegions[accumulatorRegionStackSize] = region.subregionQOverPtMaxPhi0Max();
        accumulatorRegions[accumulatorRegionStackSize].pointListBegin_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
        accumulatorRegions[accumulatorRegionStackSize].pointListEnd_ = accumulatorRegions[accumulatorRegionStackSize - 1].pointListEnd_;
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
        addSolution(result, region, regionSolverData);
    }
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

bool HelixSolver::regionHit(const AccumulatorRegion& region, float r, float phi)
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

void HelixSolver::addSolution(Result& result, const AccumulatorRegion& region, RegionSolverData& regionSolverData)
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

void HelixSolver::rotateSolutions(Result& result, u_int32_t regionSolutionsBegin)
{
    // Rotate solutions by pi
    const u_int32_t numSolutions = result.numSolutions_;
    for (u_int32_t i = regionSolutionsBegin; i < numSolutions; ++i)
    {
        result.solutionPhis_[i] = wrapMinusPiToPi(result.solutionPhis_[i] + M_PI);
    }
}
