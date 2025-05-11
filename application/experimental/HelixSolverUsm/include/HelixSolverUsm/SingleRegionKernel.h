#pragma once

#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "HelixSolverUsm/SingleRegionKernelMemory.h"

#include <sycl/sycl.hpp>
#include <cmath>
#include <gtest/gtest_prod.h>


class SingleRegionKernel
{
public:
    SingleRegionKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result, const SingleRegionKernelMemory* memory);

    SYCL_EXTERNAL void operator()(sycl::id<1> regionIdIdx) const;

    static std::unique_ptr<DeviceResourceGroup> createResourceGroup(sycl::queue& queue);

protected:
    class AccumulatorRegion
    {
    public:
        AccumulatorRegion() = default;
        AccumulatorRegion(float qOverPtMin, float qOverPtMax, float phi0Min, float phi0Max, u_int8_t qOverPtDivisionLevel = 0, u_int8_t phi0DivisionLevel = 0)
            : qOverPtMin_(qOverPtMin)
            , qOverPtMax_(qOverPtMax)
            , phi0Min_(phi0Min)
            , phi0Max_(phi0Max)
            , qOverPtDivisionLevel_(qOverPtDivisionLevel)
            , phi0DivisionLevel_(phi0DivisionLevel) {}
        

        bool operator==(const AccumulatorRegion& other) const
        {
            // Note: not comparing pointListBegin and pointListEnd
            constexpr float epsilon = 1e-9f;
            return std::abs(qOverPtMin_ - other.qOverPtMin_) < epsilon
                && std::abs(qOverPtMax_ - other.qOverPtMax_) < epsilon
                && std::abs(phi0Min_ - other.phi0Min_) < epsilon
                && std::abs(phi0Max_ - other.phi0Max_) < epsilon
                && qOverPtDivisionLevel_ == other.qOverPtDivisionLevel_
                && phi0DivisionLevel_ == other.phi0DivisionLevel_;
        }

        inline AccumulatorRegion subregionQOverPtMinPhi0Min() const
        {
            return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
        }

        inline AccumulatorRegion subregionQOverPtMinPhi0Max() const
        {
            return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
        }

        inline AccumulatorRegion subregionQOverPtMaxPhi0Min() const
        {
            return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
        }

        inline AccumulatorRegion subregionQOverPtMaxPhi0Max() const
        {
            return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
        }

        inline AccumulatorRegion subregionQOverPtMin() const
        {
            return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), phi0Min_, phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_);
        }

        inline AccumulatorRegion subregionQOverPtMax() const
        {
            return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, phi0Min_, phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_);
        }

        inline AccumulatorRegion subregionPhi0Min() const
        {
            return AccumulatorRegion(qOverPtMin_, qOverPtMax_, phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_, phi0DivisionLevel_ + 1);
        }

        inline AccumulatorRegion subregionPhi0Max() const
        {
            return AccumulatorRegion(qOverPtMin_, qOverPtMax_, 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_, phi0DivisionLevel_ + 1);
        }

        float qOverPtMin_;
        float qOverPtMax_;
        float phi0Min_;
        float phi0Max_;
        u_int8_t qOverPtDivisionLevel_;
        u_int8_t phi0DivisionLevel_;
        u_int32_t pointListBegin_;
        u_int32_t pointListEnd_;
    };

    const SplitterSettings* deviceSplitterSettings_;
    const u_int32_t* deviceNumPoints_;
    const float* deviceXs_;
    const float* deviceYs_;
    const float* deviceZs_;
    const EventUsm::LayerNumber* deviceLayers_;

    u_int32_t* deviceNumSolutions_;
    u_int32_t* deviceRegionNumSolutions_;
    u_int8_t* deviceSolutionHitCounts_;
    float* deviceSolutionRs_;
    float* deviceSolutionPhis_;

    const EventUsm* event_; // TODO: Remove
    const ResultUsm* result_;    // TODO: Remove

    u_int32_t* kernelIndexes_;
    float* kernelXs_;
    float* kernelYs_;
    float* kernelZs_;
    EventUsm::LayerNumber* kernelLayers_;
    u_int32_t* kernelPointLists_;
    float* kernelRs_;
    float* kernelPhis_;

    // Based on thesis p. 22 Phi_0 is in range dependent on region
    // Phi_min and Phi_max are region dependent
    static constexpr float SpaceMaxPhiPhi0AbsDiff = 0.42f;  // TODO: Tune
    static constexpr float SpaceMinQOverPt = 0.0f;   // TODO: I have no idea what this value should be, tune
    static constexpr float SpaceMaxQOverPt = 0.0005f;    // TODO: I have no idea what this value should be, tune
    static constexpr u_int8_t Phi0MaxDivisionLevel = SingleRegionKernelMemory::Phi0MaxDivisionLevel;
    static constexpr u_int8_t QOverPtMaxDivisionLevel = SingleRegionKernelMemory::QOverPtMaxDivisionLevel;
    static constexpr u_int8_t MaxDivisionLevel = SingleRegionKernelMemory::MaxDivisionLevel;
    static constexpr u_int8_t MaxAccumulatorRegionStackSize = SingleRegionKernelMemory::MaxAccumulatorRegionStackSize;
    static constexpr u_int8_t MaxPointListsNum = SingleRegionKernelMemory::MaxPointListsNum;
    static constexpr u_int16_t MaxPointsInRegion = SingleRegionKernelMemory::MaxPointsInRegion;
    static constexpr u_int32_t MaxPointListsPointsNum = SingleRegionKernelMemory::MaxPointListsPointsNum;
    static constexpr float BMagnitude = 2.0f;   // TODO: Tune
    static constexpr u_int8_t SolutionHitsThreshold = 8;    // TODO: Tune
    static constexpr u_int8_t LinesCrossingsThreshold = 3;   // TODO: Tune

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

    SYCL_EXTERNAL void filterPointsInRegion(const Splitter& splitter, u_int16_t regionId, u_int32_t& numPoints, u_int32_t* indexes, float* xs, float* ys, float* zs, EventUsm::LayerNumber* layers) const;
    void convertToPolarCoordinates(float* phis, float* rs, const float* xs, const float* ys, u_int32_t numPoints) const;
    void rotateRegionAndPoints(float& regionPhi0Min, float& regionPhi0Max, float* phis, u_int32_t numPoints) const;
    static void processNextAccumulatorRegion(u_int16_t regionId, AccumulatorRegion* accumulatorRegions, u_int8_t& accumulatorRegionStackSize, u_int32_t* pointLists, const u_int32_t* indexes, const float* rs, const float* phis, uint32_t* regionNumSolutions, u_int8_t* solutionHitCounts, float* solutionRs, float* solutionPhis, const uint8_t solutionHitsThreshold, const uint8_t linesCrossingsThreshold);
    static bool enoughHitsAndLinesCrossings(const AccumulatorRegion& region, const float* rs, const float* phis, const u_int32_t* pointLists, const u_int8_t solutionHitsThreshold, const u_int8_t linesCrossingsThreshold);
    static void fillNewPointList(AccumulatorRegion& region, const AccumulatorRegion& sourceRegion, u_int32_t* pointLists, const float* rs, const float* phis);
    static bool regionHit(float qOverPtMin, float qOverPtMax, float phi0Min, float phi0Max, float r, float phi);
    static void addSolution(u_int16_t regionId, const AccumulatorRegion& region, uint32_t* regionNumSolutions, u_int8_t* solutionHitCounts, float* solutionRs, float* solutionPhis);
    void rotateSolutions(u_int16_t regionId) const;

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
    friend class SingleHelixDetectionTest;
};