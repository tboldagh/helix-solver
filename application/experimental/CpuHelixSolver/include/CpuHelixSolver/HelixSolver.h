#pragma once

#include "CpuHelixSolver/Event.h"
#include "CpuHelixSolver/Result.h"
#include "CpuHelixSolver/Task.h"
#include "CpuHelixSolver/AccumulatorRegion.h"
#include "SplitterUsm/Splitter.h"

#include <cmath>
#include <gtest/gtest_prod.h>


class HelixSolver
{
public:
    HelixSolver(const Splitter& splitter);

    void solve(Task& task);
    void setSplitter(const Splitter& splitter) { splitter_ = splitter; }

    // Auxiliary functions, move somewhere else later
    static float wrapMinusPiToPi(float angle)
    {
        return std::fmod(angle + M_PI, 2.0f * M_PI) + (angle < -M_PI ? 1.0f : -1.0f) * M_PI;
    }

    static float angleWrap2Pi(float angle)
    {
        return std::fmod(angle, 2 * M_PI) + (angle >= 0 ? 0 : 2 * M_PI);
    }

    static float atan2Wrap2Pi(float y, float x)
    {
        return angleWrap2Pi(std::atan2(y, x));
    }

private:
    class RegionSolverData
    {
    public:
        static constexpr u_int16_t MaxPointsInRegion = 10000;  // TODO: Tune
        static constexpr u_int8_t Phi0MaxDivisionLevel = 11;
        static constexpr u_int8_t QOverPtMaxDivisionLevel = 9;
        static constexpr u_int8_t MaxDivisionLevel = std::max(Phi0MaxDivisionLevel, QOverPtMaxDivisionLevel);
        static constexpr u_int8_t MaxAccumulatorRegionStackSize = MaxDivisionLevel * 4;
        static constexpr u_int8_t MaxPointListsNum = MaxDivisionLevel + 2;
        static constexpr u_int32_t MaxPointListsPointsNum = MaxPointsInRegion * MaxPointListsNum / 4;   // TODO: This is max possible number of points in all lists combined. Can be tuned
        static constexpr u_int8_t LayerHitThreshold = 3;

        u_int16_t regionId_;
        float xAngleMin_;
        float xAngleMax_;
        u_int8_t regionSolutionHitsThreshold_;
        u_int8_t regionLinesCrossingsThreshold_;
        u_int8_t regionSkipCrossingsCheckThreshold_;
        u_int32_t numPoints_;
        u_int32_t indexes_[MaxPointsInRegion];
        float rs_[MaxPointsInRegion];
        float phis_[MaxPointsInRegion];
        u_int8_t layers_[MaxPointsInRegion];
        AccumulatorRegion accumulatorRegions_[MaxAccumulatorRegionStackSize];
        u_int8_t accumulatorRegionStackSize_;
        float pointListsRs_[MaxPointListsPointsNum];
        float pointListsPhis_[MaxPointListsPointsNum];
        u_int8_t pointListsLayers_[MaxPointListsPointsNum];
    };

    void solveRegion(Task& task, RegionSolverData& regionSolverData);
    void convertToPolarCoordinates(const Event& event, RegionSolverData& regionSolverData);
    void assignLayers(const Event& event, RegionSolverData& regionSolverData);
    void rotateRegionAndPoints(float& regionPhi0Min, float& regionPhi0Max, RegionSolverData& regionSolverData);
    void processNextAccumulatorRegion(Result& result, RegionSolverData& regionSolverData);
    void fillNewPointList(AccumulatorRegion& region, const AccumulatorRegion& sourceRegion, RegionSolverData& regionSolverData);
    bool regionHit(const AccumulatorRegion& region, const float r, const float phi);
    bool enoughLayerHits(const AccumulatorRegion& region, const RegionSolverData& regionSolverData);
    bool enoughHitsAndLinesCrossing(const AccumulatorRegion& region, const RegionSolverData& regionSolverData);
    void addSolution(Result& result, const AccumulatorRegion& region, const RegionSolverData& regionSolverData);
    void rotateSolutions(Result& result, const u_int32_t regionSolutionsBegin);

    // Based on thesis p. 22 Phi_0 is in range dependent on region
    // Phi_min and Phi_max are region dependent
    static constexpr float SpaceMaxPhiPhi0AbsDiff = M_PI / 4;
    static constexpr float SpaceMinQOverPt = -0.31f;
    static constexpr float SpaceMaxQOverPt = 0.31f;
    static constexpr u_int8_t SolutionHitsThreshold = 7;
    static constexpr u_int8_t LinesCrossingsThreshold = 3;
    static constexpr u_int8_t SkipCrossingsCheckThreshold = 20;    // TODO: Tune
    static constexpr float BMagnitude = 2.0f;

    Splitter splitter_;
    std::vector<RegionSolverData> regionSolverData_;

    FRIEND_TEST(RegionHitTest, AboveRegion);
    FRIEND_TEST(RegionHitTest, TopRightCorner);
    FRIEND_TEST(RegionHitTest, CrossingRight);
    FRIEND_TEST(RegionHitTest, BottomRightCorner);
    FRIEND_TEST(RegionHitTest, TopLeftAndBottomRightCorner);
    FRIEND_TEST(RegionHitTest, AboveLeftAndBelowRight);
    FRIEND_TEST(RegionHitTest, TopLeftCorner);
    FRIEND_TEST(RegionHitTest, CrossingLeft);
    FRIEND_TEST(RegionHitTest, CrossingLeftAndRight);
    FRIEND_TEST(RegionHitTest, BottomLeftCorner);
    FRIEND_TEST(RegionHitTest, BelowRegion);
    friend class FillNewPointListTest;
    FRIEND_TEST(FillNewPointListTest, SinglePointInRegion);
    FRIEND_TEST(FillNewPointListTest, SinglePointNotInRegion);
    FRIEND_TEST(FillNewPointListTest, TwoPointsOneInRegion);
    friend class FillNewPointListMultiplePointsTest;
    FRIEND_TEST(FillNewPointListMultiplePointsTest, Basic);
    FRIEND_TEST(FillNewPointListMultiplePointsTest, NewListNotJustAfterSource);
    FRIEND_TEST(FillNewPointListMultiplePointsTest, SourceNotAtBegin);
    friend class ProcessNextAccumulatorRegionTest;
    FRIEND_TEST(ProcessNextAccumulatorRegionTest, DropRegionIfNumberOfPointsIsBelowThreshold);
    FRIEND_TEST(ProcessNextAccumulatorRegionTest, DivideInBothDimensionsIfMaxDivisionLevelsNotReached);
    FRIEND_TEST(ProcessNextAccumulatorRegionTest, DividePhi0IfMaxDivisionLevelsNotReached);
    FRIEND_TEST(ProcessNextAccumulatorRegionTest, DivideQOverPtIfMaxDivisionLevelsNotReached);
    FRIEND_TEST(ProcessNextAccumulatorRegionTest, AddSolutionIfMaxDivisionLevelsReached);
    friend class HelixDetectionTest;
    FRIEND_TEST(HelixDetectionTest, SingleHelixBasicWedge);
    FRIEND_TEST(HelixDetectionTest, SingleHelixRotatedWedge);
    FRIEND_TEST(SingleCounterClockwiseHelixInCenterOfWedgeTest, CounterClockwise);
    FRIEND_TEST(SingleClockwiseHelixInCenterOfWedgeTest, Clockwise);
    FRIEND_TEST(HelixDetectionTest, MultipleHelixesBasicWedge);
    FRIEND_TEST(HelixDetectionTest, MultipleHelixesRotatedWedge);
    FRIEND_TEST(MultipleCounterClockwiseHelixInCenterOfWedgeTest, CounterClockwise);
    FRIEND_TEST(MultipleClockwiseHelixInCenterOfWedgeTest, Clockwise);
    FRIEND_TEST(HelixDetectionTest, FullEvent);
};