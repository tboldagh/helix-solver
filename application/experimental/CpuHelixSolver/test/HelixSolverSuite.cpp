#include "CpuHelixSolver/HelixSolver.h"
#include "SplitterUsm/Splitter.h"
#include "CpuHelixSolver/Event.h"
#include "CpuHelixSolver/Result.h"
#include "CpuHelixSolver/Task.h"
#include "Logger/OstreamLogger.h"
#include "Logger/Logger.h"
#include "RootEventLoader/RootEventLoader.h"
#include "SpacepointsGenerator/SpacepointsGenerator.h"
#include "DataTypes/Spacepoint.h"
#include "DataTypes/ParticleInitial.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

class HelixSolverTest : public ::testing::Test
{
protected:
    HelixSolverTest()
    : splitter_(getSplitterSettings())
    , helixSolver_(splitter_)
    , task_(event_, result_)
    {
        Logger::ILogger::setGlobalInstance(&logger_);
    }

    ~HelixSolverTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    static SplitterSettings getSplitterSettings()
    {
        constexpr float maxAbsXy = 1100.0;
        constexpr float maxAbsZ = 3100.0;
        constexpr float minZAngle = 0.0;
        constexpr float maxZAngle = 2.0 * M_PI;
        constexpr float minXAngle = 1.0 / 16 * M_PI;
        constexpr float maxXAngle = 15.0 / 16 * M_PI;
        constexpr float poleRegionAngle = 1.0 / 16 * M_PI;
        constexpr float interactionRegionMin = -200.0;
        constexpr float interactionRegionMax = 200.0;
        constexpr float zAngleMargin = 4.0 / 256 * M_PI;
        constexpr float xAngleMargin = 2.0 / 256 * M_PI;
        constexpr u_int8_t numZRanges = 16;
        constexpr u_int8_t numXRanges = 16;
        constexpr float filterOutCenterR = 150.0;
        constexpr float filterOutCenterZ = 500.0;
        return SplitterSettings(
            maxAbsXy, maxAbsZ,
            minZAngle, maxZAngle,
            minXAngle, maxXAngle,
            poleRegionAngle,
            interactionRegionMin, interactionRegionMax,
            zAngleMargin, xAngleMargin,
            numZRanges, numXRanges,
            filterOutCenterR, filterOutCenterZ
        );
    }

    static void writeRegion(u_int32_t* destination, const std::vector<u_int32_t>& source)
    {
        for (u_int32_t i = 0; i < source.size(); ++i)
        {
            destination[i] = source[i];
        }
    }

    static void assertRegionEq(const u_int32_t* actual, const std::vector<u_int32_t>& expected, u_int32_t begin, u_int32_t end)
    {
        for (u_int32_t i = 0; i < end - begin; ++i)
        {
            EXPECT_EQ(actual[begin + i], expected[i]);
        }
    }

    Logger::OstreamLogger logger_{std::cout};

    Splitter splitter_;
    HelixSolver helixSolver_;
    Event event_;
    Result result_;
    Task task_;
};

class RegionHitTest : public HelixSolverTest
{
protected:
    RegionHitTest() = default;
    ~RegionHitTest() override = default;

    static constexpr float qOverPtMin = 1.0f;
    static constexpr float qOverPtMax = 2.0f;
    static constexpr float phi0Min = 1.0f;
    static constexpr float phi0Max = 2.0f;
    static constexpr u_int8_t qOverPtDivisionLevel = 1;
    static constexpr u_int8_t phi0DivisionLevel = 1;
    AccumulatorRegion region_{qOverPtMin, qOverPtMax, phi0Min, phi0Max, qOverPtDivisionLevel, phi0DivisionLevel};
};

TEST_F(RegionHitTest, AboveRegion)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 5.0f;

    EXPECT_FALSE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, TopRightCorner)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 4.0f - 1e-6f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, CrossingRight)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 3.5f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, BottomRightCorner)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 2.0f;
    constexpr float phi = 5.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, TopLeftAndBottomRightCorner)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 3.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, AboveLeftAndBelowRight)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 4.0f;
    constexpr float phi = 8.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, TopLeftCorner)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 2.0f;
    constexpr float phi = 4.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, CrossingLeft)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 2.5f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, CrossingLeftAndRight)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 0.5f;
    constexpr float phi = 2.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, BottomLeftCorner)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 2.0f;

    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));
}

TEST_F(RegionHitTest, BelowRegion)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 1.0f;

    EXPECT_FALSE(helixSolver_.regionHit(region_, r, phi));
}


class FillNewPointListTest : public RegionHitTest
{
protected:
    FillNewPointListTest()
    {
        regionSolverData_.numPoints_ = 0;

        // Fill with dummy values make sure the results are not accidental
        for (u_int32_t i = 0; i < HelixSolver::RegionSolverData::MaxPointListsPointsNum; ++i)
        {
            regionSolverData_.pointLists_[i] = 2137;
        }
    }

    ~FillNewPointListTest() override = default;
    
    HelixSolver::RegionSolverData regionSolverData_;
};


TEST_F(FillNewPointListTest, SinglePointInRegion)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 3.0f;
    EXPECT_TRUE(helixSolver_.regionHit(region_, r, phi));

    regionSolverData_.rs_[0] = r;
    regionSolverData_.phis_[0] = phi;
    regionSolverData_.pointLists_[0] = 0;

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 0;
    sourceRegion.pointListEnd_ = 1;
    region_.pointListBegin_ = sourceRegion.pointListEnd_;
    region_.pointListEnd_ = sourceRegion.pointListEnd_;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, sourceRegion.pointListEnd_);
    EXPECT_EQ(region_.pointListEnd_, 2);
    EXPECT_EQ(regionSolverData_.pointLists_[1], 0);
}

TEST_F(FillNewPointListTest, SinglePointNotInRegion)
{
    constexpr float r = 2.0f / HelixSolver::BMagnitude * 1.0f;
    constexpr float phi = 10.0f;
    EXPECT_FALSE(helixSolver_.regionHit(region_, r, phi));

    regionSolverData_.rs_[0] = r;
    regionSolverData_.phis_[0] = phi;
    regionSolverData_.pointLists_[0] = 0;

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 0;
    sourceRegion.pointListEnd_ = 1;
    region_.pointListBegin_ = sourceRegion.pointListEnd_;
    region_.pointListEnd_ = sourceRegion.pointListEnd_;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, sourceRegion.pointListEnd_);
    EXPECT_EQ(region_.pointListEnd_, 1);
}

TEST_F(FillNewPointListTest, TwoPointsOneInRegion)
{
    // First point - not in region
    regionSolverData_.rs_[0] = 2.0f / HelixSolver::BMagnitude * 1.0f;
    regionSolverData_.phis_[0] = 10.0f;
    EXPECT_FALSE(helixSolver_.regionHit(region_, regionSolverData_.rs_[0], regionSolverData_.phis_[0]));

    // Second point - in region
    regionSolverData_.rs_[1] = 2.0f / HelixSolver::BMagnitude * 1.0f;
    regionSolverData_.phis_[1] = 3.0f;
    EXPECT_TRUE(helixSolver_.regionHit(region_, regionSolverData_.rs_[1], regionSolverData_.phis_[1]));

    // Set up source region with both points
    const std::vector<u_int32_t> sourcePointList = {0, 1};
    writeRegion(regionSolverData_.pointLists_, sourcePointList);

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 0;
    sourceRegion.pointListEnd_ = 2;
    region_.pointListBegin_ = sourceRegion.pointListEnd_;
    region_.pointListEnd_ = sourceRegion.pointListEnd_;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, sourceRegion.pointListEnd_);
    EXPECT_EQ(region_.pointListEnd_, 3);
    assertRegionEq(regionSolverData_.pointLists_, {1}, 2, 3);
}

class FillNewPointListMultiplePointsTest : public FillNewPointListTest
{
protected:
    FillNewPointListMultiplePointsTest()
    {
        for (float phi : inRegionPhis_)
        {
            EXPECT_TRUE(helixSolver_.regionHit(region_, r_, phi));
        }

        for (float phi : notInRegionPhis_)
        {
            EXPECT_FALSE(helixSolver_.regionHit(region_, r_, phi));
        }

        for (unsigned i = 0; i < inRegionIndexes_.size(); ++i)
        {
            regionSolverData_.rs_[inRegionIndexes_[i]] = r_;
            regionSolverData_.phis_[inRegionIndexes_[i]] = inRegionPhis_[i];
        }

        for (unsigned i = 0; i < notInRegionIndexes_.size(); ++i)
        {
            regionSolverData_.rs_[notInRegionIndexes_[i]] = r_;
            regionSolverData_.phis_[notInRegionIndexes_[i]] = notInRegionPhis_[i];
        }
    }

    ~FillNewPointListMultiplePointsTest() override = default;

    const float r_ = 2.0f / HelixSolver::BMagnitude * 1.0f;
    const std::vector<float> inRegionPhis_ = { 2.5f, 2.75f, 3.0f, 3.25f, 3.5f};
    const std::vector<float> notInRegionPhis_ = { 0.0f, 1.0f, 5.0f, 6.0f, 7.0f};
    // Mixed on purpose
    const std::vector<u_int32_t> inRegionIndexes_ = {0, 3, 4, 8, 9};
    const std::vector<u_int32_t> notInRegionIndexes_ = {1, 2, 5, 6, 7};
};

TEST_F(FillNewPointListMultiplePointsTest, Basic)
{
    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(regionSolverData_.pointLists_, sourcePointList);

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 0;
    sourceRegion.pointListEnd_ = 10;
    region_.pointListBegin_ = sourceRegion.pointListEnd_;
    region_.pointListEnd_ = sourceRegion.pointListEnd_;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, sourceRegion.pointListEnd_);
    EXPECT_EQ(region_.pointListEnd_, 15);
    assertRegionEq(regionSolverData_.pointLists_, sourcePointList, 0, 10);
    assertRegionEq(regionSolverData_.pointLists_, inRegionIndexes_, 10, 15);
}

TEST_F(FillNewPointListMultiplePointsTest, NewListNotJustAfterSource)
{
    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(regionSolverData_.pointLists_, sourcePointList);

    const std::vector<u_int32_t> unrelatedPointList = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    writeRegion(&regionSolverData_.pointLists_[10], unrelatedPointList);
    writeRegion(&regionSolverData_.pointLists_[20], unrelatedPointList);

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 0;
    sourceRegion.pointListEnd_ = 10;
    region_.pointListBegin_ = 30;    // source + 2 unrelated
    region_.pointListEnd_ = 30;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, 30);
    EXPECT_EQ(region_.pointListEnd_, 35);
    assertRegionEq(regionSolverData_.pointLists_, sourcePointList, 0, 10);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, unrelatedPointList, 10, 20);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, unrelatedPointList, 20, 30);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, inRegionIndexes_, 30, 35);
}

TEST_F(FillNewPointListMultiplePointsTest, SourceNotAtBegin)
{
    const std::vector<u_int32_t> unrelatedPointList = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    writeRegion(regionSolverData_.pointLists_, unrelatedPointList);

    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(&regionSolverData_.pointLists_[10], sourcePointList);

    writeRegion(&regionSolverData_.pointLists_[20], unrelatedPointList);

    AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin_ = 10;
    sourceRegion.pointListEnd_ = 20;
    region_.pointListBegin_ = 30;    // unrelated + source + unrelated
    region_.pointListEnd_ = 30;

    helixSolver_.fillNewPointList(region_, sourceRegion, regionSolverData_);
    EXPECT_EQ(region_.pointListBegin_, 30);
    EXPECT_EQ(region_.pointListEnd_, 35);
    assertRegionEq(regionSolverData_.pointLists_, unrelatedPointList, 0, 10);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, sourcePointList, 10, 20);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, unrelatedPointList, 20, 30);  // Assert no unwanted changes
    assertRegionEq(regionSolverData_.pointLists_, inRegionIndexes_, 30, 35);
}


class ProcessNextAccumulatorRegionTest : public FillNewPointListTest
{
protected:
    ProcessNextAccumulatorRegionTest()
    {
        regionSolverData_.regionSolutionHitsThreshold_ = HelixSolver::SolutionHitsThreshold;
        regionSolverData_.regionLinesCrossingsThreshold_ = HelixSolver::LinesCrossingsThreshold;
        regionSolverData_.regionSkipCrossingsCheckThreshold_ = HelixSolver::SolutionHitsThreshold - 1;  // always skip crossings check

        std::vector<float> rs = {0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0};
        for (u_int32_t i = 0; i < rs.size(); ++i)
        {
            regionSolverData_.rs_[i] = rs[i];
        }

        std::vector<float> phis = {2.0, 2.3, 2.6, 2.9, 3.2, 3.5, 3.8, 4.1, 4.4, 4.7};
        for (u_int32_t i = 0; i < phis.size(); ++i)
        {
            regionSolverData_.phis_[i] = phis[i];
        }

        std::vector<uint32_t> layers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        for (u_int32_t i = 0; i < layers.size(); ++i)
        {
            regionSolverData_.layers_[i] = layers[i];
        }

        const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        writeRegion(regionSolverData_.pointLists_, sourcePointList);
    }
    ~ProcessNextAccumulatorRegionTest() override = default;

    const u_int16_t regionId_ = 42;
    Result result_;
};

TEST_F(ProcessNextAccumulatorRegionTest, DropRegionIfNumberOfPointsIsBelowThreshold)
{
    region_.qOverPtDivisionLevel_ = HelixSolver::RegionSolverData::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel_ = HelixSolver::RegionSolverData::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin_ = 0;
    region_.pointListEnd_ = regionSolverData_.regionSolutionHitsThreshold_ - 1;
    regionSolverData_.accumulatorRegions_[0] = region_;
    regionSolverData_.accumulatorRegionStackSize_ = 1;

    helixSolver_.processNextAccumulatorRegion(result_, regionSolverData_);
    EXPECT_EQ(regionSolverData_.accumulatorRegionStackSize_, 0);
}

TEST_F(ProcessNextAccumulatorRegionTest, DivideInBothDimensionsIfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel_ = HelixSolver::RegionSolverData::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel_ = HelixSolver::RegionSolverData::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin_ = 0;
    region_.pointListEnd_ = regionSolverData_.regionSolutionHitsThreshold_;
    regionSolverData_.accumulatorRegions_[0] = region_;
    regionSolverData_.accumulatorRegionStackSize_ = 1;

    helixSolver_.processNextAccumulatorRegion(result_, regionSolverData_);
    EXPECT_EQ(regionSolverData_.accumulatorRegionStackSize_, 4);

    std::vector<AccumulatorRegion> actualRegions(regionSolverData_.accumulatorRegions_, regionSolverData_.accumulatorRegions_ + regionSolverData_.accumulatorRegionStackSize_);
    const float qOverPtMiddle = (qOverPtMin + qOverPtMax) / 2;
    const float phi0Middle = (phi0Min + phi0Max) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Min, phi0Middle, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_ + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Min, phi0Middle, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_ + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Middle, phi0Max, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_ + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Middle, phi0Max, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_ + 1)));
}

TEST_F(ProcessNextAccumulatorRegionTest, DividePhi0IfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel_ = HelixSolver::RegionSolverData::QOverPtMaxDivisionLevel;
    region_.phi0DivisionLevel_ = HelixSolver::RegionSolverData::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin_ = 0;
    region_.pointListEnd_ = regionSolverData_.regionSkipCrossingsCheckThreshold_ + 1;
    regionSolverData_.accumulatorRegions_[0] = region_;
    regionSolverData_.accumulatorRegionStackSize_ = 1;

    helixSolver_.processNextAccumulatorRegion(result_, regionSolverData_);
    EXPECT_EQ(regionSolverData_.accumulatorRegionStackSize_, 2);

    std::vector<AccumulatorRegion> actualRegions(regionSolverData_.accumulatorRegions_, regionSolverData_.accumulatorRegions_ + regionSolverData_.accumulatorRegionStackSize_);
    const float phi0Middle = (phi0Min + phi0Max) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMin, qOverPtMax, phi0Min, phi0Middle, region_.qOverPtDivisionLevel_, region_.phi0DivisionLevel_ + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMin, qOverPtMax, phi0Middle, phi0Max, region_.qOverPtDivisionLevel_, region_.phi0DivisionLevel_ + 1)));
}

TEST_F(ProcessNextAccumulatorRegionTest, DivideQOverPtIfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel_ = HelixSolver::RegionSolverData::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel_ = HelixSolver::RegionSolverData::Phi0MaxDivisionLevel;
    region_.pointListBegin_ = 0;
    region_.pointListEnd_ = regionSolverData_.regionSkipCrossingsCheckThreshold_ + 1;
    regionSolverData_.accumulatorRegions_[0] = region_;
    regionSolverData_.accumulatorRegionStackSize_ = 1;

    helixSolver_.processNextAccumulatorRegion(result_, regionSolverData_);
    EXPECT_EQ(regionSolverData_.accumulatorRegionStackSize_, 2);

    std::vector<AccumulatorRegion> actualRegions(regionSolverData_.accumulatorRegions_, regionSolverData_.accumulatorRegions_ + regionSolverData_.accumulatorRegionStackSize_);
    const float qOverPtMiddle = (qOverPtMin + qOverPtMax) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Min, phi0Max, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_)));
    EXPECT_THAT(actualRegions, ::testing::Contains(AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Min, phi0Max, region_.qOverPtDivisionLevel_ + 1, region_.phi0DivisionLevel_)));
}

TEST_F(ProcessNextAccumulatorRegionTest, AddSolutionIfMaxDivisionLevelsReached)
{
    region_.qOverPtDivisionLevel_ = HelixSolver::RegionSolverData::QOverPtMaxDivisionLevel;
    region_.phi0DivisionLevel_ = HelixSolver::RegionSolverData::Phi0MaxDivisionLevel;
    region_.pointListBegin_ = 0;
    region_.pointListEnd_ = regionSolverData_.regionSkipCrossingsCheckThreshold_ + 1;
    regionSolverData_.accumulatorRegions_[0] = region_;
    regionSolverData_.accumulatorRegionStackSize_ = 1;

    helixSolver_.processNextAccumulatorRegion(result_, regionSolverData_);
    EXPECT_EQ(regionSolverData_.accumulatorRegionStackSize_, 0);
    EXPECT_EQ(result_.numSolutions_, 1);
    EXPECT_EQ(result_.solutionHitCounts_[0], HelixSolver::SolutionHitsThreshold);
    const float expectedR = 1 / (0.5f * (qOverPtMin + qOverPtMax) * HelixSolver::BMagnitude);
    EXPECT_FLOAT_EQ(result_.solutionRs_[0], expectedR);
    const float expectedPhi = HelixSolver::wrapMinusPiToPi(0.5f * (phi0Min + phi0Max) + 0.5f * M_PI);
    EXPECT_FLOAT_EQ(result_.solutionPhis_[0], expectedPhi);
}


class SingleHelixDetectionTest : public FillNewPointListTest
{
protected:
    SingleHelixDetectionTest()
    : logger_(std::cout)
    {
        logger_.setMinSeverity(Logger::LogMessage::Severity::Debug);
        Logger::ILogger::setGlobalInstance(&logger_);
    }
    
    ~SingleHelixDetectionTest() override = default;

    static float lerp(float minValue, float maxValue, float t)
    {
        return minValue + t * (maxValue - minValue);
    }

    void generateSpacepoints(const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints)
    {
        constexpr float maxAbsXy = 1100.0f;
        constexpr float maxAbsZ = 3100.0f;
        SpacepointsGenerator spacepointsGenerator_(maxAbsZ, maxAbsXy);
        std::vector<DataTypes::Spacepoint> spacepoints = spacepointsGenerator_.generate(xAngles, zAngles, interactionZs, rs, counterClockwise, numPoints);
    
        for (const auto& spacepoint : spacepoints)
        {
            event_.xs_[event_.numPoints_] = spacepoint.x_;
            event_.ys_[event_.numPoints_] = spacepoint.y_;
            event_.zs_[event_.numPoints_] = spacepoint.z_;
            event_.numPoints_++;
        }
    }

    void generateSpacepointsAndSolve(const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints, u_int16_t regionId)
    {
        generateSpacepoints(xAngles, zAngles, interactionZs, rs, counterClockwise, numPoints);

        const u_int32_t eventNumPoints = event_.numPoints_;

        // for (u_int32_t i = 0; i < eventNumPoints; ++i)
        // {
        //     LOG_DEBUG("(" + std::to_string(event_.xs_[i]) + ", " + std::to_string(event_.ys_[i]) + ", " + std::to_string(event_.zs_[i]) + ")");
        // }

        regionSolverData_.regionId_ = regionId;
        regionSolverData_.numPoints_ = eventNumPoints;
        for (u_int32_t i = 0; i < eventNumPoints; ++i)
        {
            regionSolverData_.indexes_[i] = i;
        }

        helixSolver_.solveRegion(task_, regionSolverData_);
    }

    static float rToQOverPt(float r)
    {
        return 1.0f / (r * HelixSolver::BMagnitude);
    }

    static float qOverPtToR(float qOverPt)
    {
        return 1.0f / (qOverPt * HelixSolver::BMagnitude);
    }

    void saveResult(const std::string& path)
    {
        std::ofstream resultFile(path);
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            resultFile << i << ",\t" 
                    << static_cast<unsigned>(result_.solutionHitCounts_[i]) << ",\t"
                    << result_.solutionRs_[i] << ",\t"
                    << result_.solutionPhis_[i] << ",\t"
                    << rToQOverPt(result_.solutionRs_[i]) << std::endl;
        }
        resultFile.close();
    }

    bool matchingSolutionExists(float expectedR, float expectedPhi)
    {
        const float minR = expectedR - 0.05f * expectedR;
        const float maxR = expectedR + 0.05f * expectedR;
        const float minPhi = HelixSolver::wrapMinusPiToPi(expectedPhi - 0.01f);
        const float maxPhi = HelixSolver::wrapMinusPiToPi(expectedPhi + 0.01f);

        bool foundMatchingSolution = false;
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            const float r = result_.solutionRs_[i];
            const float phi = result_.solutionPhis_[i];

            // LOG_DEBUG("r: " + std::to_string(r) + ", phi: " + std::to_string(phi) + ", minR: " + std::to_string(minR) + ", maxR: " + std::to_string(maxR) + ", minPhi: " + std::to_string(minPhi) + ", maxPhi: " + std::to_string(maxPhi));

            foundMatchingSolution |= r >= minR && r <= maxR && phi >= minPhi && phi <= maxPhi;
        }

        if (!foundMatchingSolution)
        {
            LOG_WARNING("No matching solution found for r = " + std::to_string(expectedR) + ", phi = " + std::to_string(expectedPhi));
        }

        return foundMatchingSolution;
    }

    bool solutionsDoNotRepeat()
    {
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            for (u_int32_t j = i + 1; j < result_.numSolutions_; ++j)
            {
                if (result_.solutionRs_[i] == result_.solutionRs_[j] && result_.solutionPhis_[i] == result_.solutionPhis_[j])
                {
                    LOG_WARNING("Repating Solution i: " + std::to_string(i) + ", j: " + std::to_string(j) + ", r: " + std::to_string(result_.solutionRs_[i]) + ", phi: " + std::to_string(result_.solutionPhis_[i]));
                    return false;
                }
            }
        }

        return true;
    }

    void assertSolutionsCorrect(float expectedR, float expectedPhi, u_int8_t numPoints)
    {
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            EXPECT_LE(result_.solutionHitCounts_[i], numPoints);
        }
        EXPECT_TRUE(matchingSolutionExists(expectedR, expectedPhi));
    }

    Logger::OstreamLogger logger_;

    static constexpr u_int32_t eventId = 42;
    static constexpr u_int32_t resultId = 42;
};

TEST_F(SingleHelixDetectionTest, BasicWedge)
{
    const SplitterSettings& settings = getSplitterSettings();
    const SplitterSettings::Wedge wedge = settings.wedges_[1];
    const float xAngle = (wedge.xAngleMin_ + wedge.xAngleMax_) / 2;
    const float zAngle = (wedge.zAngleMin_ + wedge.zAngleMax_) / 2;
    const float interactionZ = 0.0f;
    const float r = 10000.0f;
    const bool counterClockwise = true;
    const uint8_t numPoints = 10;

    regionSolverData_.regionSolutionHitsThreshold_ = HelixSolver::SolutionHitsThreshold;
    regionSolverData_.regionLinesCrossingsThreshold_ = HelixSolver::LinesCrossingsThreshold;
    regionSolverData_.regionSkipCrossingsCheckThreshold_ = HelixSolver::SolutionHitsThreshold * 2;

    generateSpacepointsAndSolve({xAngle}, {zAngle}, {interactionZ}, {r}, {counterClockwise}, {numPoints}, wedge.id_);

    const float expectedPhi = zAngle + 0.5f * M_PI;
    EXPECT_TRUE(matchingSolutionExists(r, expectedPhi));
    EXPECT_TRUE(solutionsDoNotRepeat());
}

TEST_F(SingleHelixDetectionTest, RotatedWedge)
{
    const SplitterSettings& settings = getSplitterSettings();
    const SplitterSettings::Wedge wedge = settings.wedges_[0];
    const float xAngle = (wedge.xAngleMin_ + wedge.xAngleMax_) / 2;
    const float zAngle = (2 * M_PI - wedge.zAngleMin_ + wedge.zAngleMax_) / 2;
    const float interactionZ = 0.0f;
    const float r = 10000.0f;
    const bool counterClockwise = true;
    const uint8_t numPoints = 10;

    regionSolverData_.regionSolutionHitsThreshold_ = HelixSolver::SolutionHitsThreshold;
    regionSolverData_.regionLinesCrossingsThreshold_ = HelixSolver::LinesCrossingsThreshold;
    regionSolverData_.regionSkipCrossingsCheckThreshold_ = HelixSolver::SolutionHitsThreshold * 2;

    generateSpacepointsAndSolve({xAngle}, {zAngle}, {interactionZ}, {r}, {counterClockwise}, {numPoints}, wedge.id_);

    const float expectedPhi = zAngle + 0.5f * M_PI;
    EXPECT_TRUE(matchingSolutionExists(r, expectedPhi));
    EXPECT_TRUE(solutionsDoNotRepeat());
}

class SingleCounterClockwiseHelixInCenterOfWedgeTest : public SingleHelixDetectionTest, public ::testing::WithParamInterface<int> {};
TEST_P(SingleCounterClockwiseHelixInCenterOfWedgeTest, CounterClockwise)
{
    const SplitterSettings& settings = getSplitterSettings();
    const SplitterSettings::Wedge wedge = settings.wedges_[GetParam()];
    const float xAngle = (wedge.xAngleMin_ + wedge.xAngleMax_) / 2;
    const float zAngle = wedge.zAngleMin_ < wedge.zAngleMax_ ? (wedge.zAngleMin_ + wedge.zAngleMax_) / 2 : (2 * M_PI - wedge.zAngleMin_ + wedge.zAngleMax_) / 2;
    const float interactionZ = 0.0f;
    const float r = 10000.0f;
    const bool counterClockwise = true;
    const uint8_t numPoints = 10;

    regionSolverData_.regionSolutionHitsThreshold_ = HelixSolver::SolutionHitsThreshold;
    regionSolverData_.regionLinesCrossingsThreshold_ = HelixSolver::LinesCrossingsThreshold;
    regionSolverData_.regionSkipCrossingsCheckThreshold_ = HelixSolver::SolutionHitsThreshold * 2;

    generateSpacepointsAndSolve({xAngle}, {zAngle}, {interactionZ}, {r}, {counterClockwise}, {numPoints}, wedge.id_);

    const float expectedPhi = zAngle + 0.5f * M_PI;
    EXPECT_TRUE(matchingSolutionExists(r, expectedPhi));
    EXPECT_TRUE(solutionsDoNotRepeat());
}
INSTANTIATE_TEST_SUITE_P(CounterClockwiseParticle, SingleCounterClockwiseHelixInCenterOfWedgeTest, ::testing::Range(0, 256));

// class SingleClockwiseHelixInCenterOfWedgeTest : public SingleHelixDetectionTest, public ::testing::WithParamInterface<int> {};
// TEST_P(SingleClockwiseHelixInCenterOfWedgeTest, Clockwise)
// {
//     const SplitterSettings& settings = getSplitterSettings();
//     const SplitterSettings::Wedge wedge = settings.wedges_[GetParam()];
//     const float xAngle = (wedge.xAngleMin_ + wedge.xAngleMax_) / 2;
//     const float zAngle = wedge.zAngleMin_ < wedge.zAngleMax_ ? (wedge.zAngleMin_ + wedge.zAngleMax_) / 2 : (2 * M_PI - wedge.zAngleMin_ + wedge.zAngleMax_) / 2;
//     const float interactionZ = 0.0f;
//     const float r = 10000.0f;
//     const bool counterClockwise = false;
//     const uint8_t numPoints = 2 * HelixSolver::SolutionHitsThreshold;

//     regionSolverData_.regionSolutionHitsThreshold_ = HelixSolver::SolutionHitsThreshold;
//     regionSolverData_.regionLinesCrossingsThreshold_ = HelixSolver::LinesCrossingsThreshold;
//     regionSolverData_.regionSkipCrossingsCheckThreshold_ = HelixSolver::SolutionHitsThreshold * 2;

//     generateSpacepointsAndSolve({xAngle}, {zAngle}, {interactionZ}, {r}, {counterClockwise}, {numPoints}, wedge.id_);

//     const float expectedPhi = zAngle + 0.5f * M_PI;
//     EXPECT_TRUE(matchingSolutionExists(r, expectedPhi));
//     EXPECT_TRUE(solutionsDoNotRepeat());
// }
// INSTANTIATE_TEST_SUITE_P(ClockwiseParticle, SingleClockwiseHelixInCenterOfWedgeTest, ::testing::Range(0, 256));

class MultipleHelixDetectionTest : public SingleHelixDetectionTest
{
protected:
    MultipleHelixDetectionTest() = default;
    ~MultipleHelixDetectionTest() override = default;

    // class Helix
    // {
    // public:
    //     Helix(float r, float phi, float xAngle, u_int8_t numPoints)
    //     : r_(r), phi_(phi), xAngle_(xAngle), numPoints_(numPoints) {}

    //     float r_;
    //     float phi_;
    //     float xAngle_;
    //     u_int8_t numPoints_;
    // };

    // void createHelixPoints(const std::vector<Helix>& helixes)
    // {
    //     for (const Helix& helix : helixes)
    //     {
    //         SingleHelixDetectionTest::createHelixPoints(helix.r_, helix.phi_, helix.xAngle_, helix.numPoints_);
    //     }
    // }

    // void createRunAndExtractResult(u_int16_t regionIndex, const std::vector<Helix>& helixes)
    // {
    //     // Create event
    //     createHelixPoints(helixes);

    //     // for (u_int32_t i = 0; i < numPoints; ++i)
    //     // {
    //     //     std::cout << "\t(" << event_.xs_[i] << ", " << event_.ys_[i] << ", " << event_.zs_[i] << ")," << std::endl;
    //     // }

    //     regionSolverData_.regionId_ = regionIndex + 1;
    //     helixSolver_.solveRegion(task_, regionSolverData_);
    // }
};

// TEST_F(MultipleHelixDetectionTest, Basic)
// {
//     constexpr u_int16_t regionIndex = 1;
//     const std::vector<Helix> helixes = {
//         Helix(2000.0f, 1.9f, 0.8f, 12),
//         Helix(2000.0f, 2.0f, 0.6f, 12),
//         Helix(2000.0f, 2.1f, 0.5f, 12),
//         Helix(5000.0f, 1.9f, 0.8f, 10),
//         Helix(5000.0f, 2.0f, 0.4f, 10),
//         Helix(5000.0f, 2.1f, 0.8f, 10),
//         Helix(10000.0f, 1.9f, 0.8f, 12),
//         Helix(10000.0f, 2.0f, 0.4f, 12),
//         Helix(10000.0f, 2.1f, 0.1f, 8),
//         Helix(20000.0f, 2.0f, 0.2f, 10),
//         Helix(20000.0f, 2.1f, 0.7f, 10),
//         Helix(20000.0f, 2.2f, 0.8f, 10)
//     };

//     createRunAndExtractResult(regionIndex, helixes);
    
//     // saveResult("/tmp/ut_sandbox/result_MultipleHelixDetectionTest_Basic.csv");

//     bool allHelixesFound = true;
//     for (const Helix& helix : helixes)
//     {
//         allHelixesFound &= matchingSolutionExists(helix.r_, helix.phi_);
//     }
//     EXPECT_TRUE(allHelixesFound);
// }

// TEST_F(MultipleHelixDetectionTest, Rotated)
// {
//     constexpr u_int16_t regionIndex = 0;
//     const std::vector<Helix> helixes = {
//         Helix(2000.0f, 1.5f, 0.8f, 12),
//         Helix(2000.0f, 1.6f, 0.5f, 12),
//         Helix(2000.0f, 1.7f, 0.2f, 12),
//         Helix(5000.0f, 1.5f, 0.8f, 10),
//         Helix(5000.0f, 1.6f, 0.4f, 8),
//         Helix(5000.0f, 1.7f, 0.9f, 10),
//         Helix(10000.0f, 1.6f, 0.8f, 12),
//         Helix(10000.0f, 1.6f, 0.4f, 12),
//         Helix(10000.0f, 1.7f, 0.1f, 8),
//         Helix(20000.0f, 1.6f, 0.2f, 10),
//         Helix(20000.0f, 1.7f, 0.7f, 10),
//         Helix(20000.0f, 1.8f, 0.8f, 12)
//     };

//     createRunAndExtractResult(regionIndex, helixes);
    
//     // saveResult("/tmp/ut_sandbox/result_MultipleHelixDetectionTest_Rotated.csv");

//     bool allHelixesFound = true;
//     for (const Helix& helix : helixes)
//     {
//         allHelixesFound &= matchingSolutionExists(helix.r_, helix.phi_);
//     }
//     EXPECT_TRUE(allHelixesFound);
// }

TEST_F(MultipleHelixDetectionTest, FullEvent)
{
    // This test assumes that full event contains at least one helix candidate for each region.
    // Goal is to assert that the kernel is able to find helixes in all regions. There is no
    // guarantee that the found helixes are correct.

    logger_.setMinSeverity(Logger::LogMessage::Severity::Debug);

    // Set wedges solution hits threshold and lines crossings threshold based on xAngles
    for (u_int16_t regionId = 1; regionId <= splitter_.settings_.wedges_.getSize(); ++regionId)
    {
        SplitterSettings::Wedge& region = splitter_.settings_.wedges_[regionId - 1];

        if (region.xAngleMin_ < 0.2f || region.xAngleMin_ > 2.5f)
        {
            region.solutionHitsThreshold_ = 12;
            region.linesCrossingsThreshold_ = 5;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 0.8f || region.xAngleMin_ > 2.0f)
        {
            region.solutionHitsThreshold_ = 10;
            region.linesCrossingsThreshold_ = 3;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 1.0f || region.xAngleMin_ > 1.8f)
        {
            region.solutionHitsThreshold_ = 8;
            region.linesCrossingsThreshold_ = 3;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else if (region.xAngleMin_ < 1.4f || region.xAngleMin_ > 1.6f)
        {
            region.solutionHitsThreshold_ = 8;
            region.linesCrossingsThreshold_ = 3;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
        else
        {
            region.solutionHitsThreshold_ = 3;
            region.skipCrossingsCheckThreshold_ = 8 * region.solutionHitsThreshold_;
        }
    }
    helixSolver_.setSplitter(splitter_);

    const std::string eventPath = "/helix/repo/data/odd_output_ttbar_PU200_100/spacepoints.root";
    RootEventLoader eventLoader;
    if (!eventLoader.setInputFile(eventPath))
    {
        ASSERT_TRUE(false);
    }
    
    constexpr EventUsm::EventId eventId = 0;
    event_.eventId_ = eventId;
    ASSERT_TRUE(eventLoader.loadEvent(eventId, event_.xs_, event_.ys_, event_.zs_, &event_.numPoints_));

    const u_int16_t numWedges = splitter_.getNumRegions() - 2;
    std::vector<u_int32_t*> regionIndexes;
    regionIndexes.reserve(numWedges);
    std::vector<u_int32_t*> regionNumPoints;
    regionNumPoints.reserve(numWedges);

    std::vector<HelixSolver::RegionSolverData> regionSolverData;
    regionSolverData.reserve(numWedges);
    for (u_int16_t i = 0; i < numWedges; ++i)
    {
        regionSolverData.emplace_back();
    }
    for (u_int16_t i = 0; i < numWedges; ++i)
    {
        regionIndexes.emplace_back(regionSolverData[i].indexes_);
        regionNumPoints.emplace_back(&regionSolverData[i].numPoints_);
    }
    splitter_.splitIntoRegions(event_.xs_, event_.ys_, event_.zs_, event_.numPoints_, regionIndexes, regionNumPoints, numWedges);

    // Reset number of solutions
    result_.numSolutions_ = 0;

    uint32_t lastRegionNumSolutions_ = 0;
    bool allRegionsContainHelix = true;
    for (u_int16_t regionId = 1; regionId <= splitter_.settings_.wedges_.getSize(); ++regionId)
    {
        regionSolverData[regionId - 1].regionId_ = regionId;
        helixSolver_.solveRegion(task_, regionSolverData[regionId - 1]);

        SplitterSettings::Wedge region = splitter_.settings_.wedges_[regionId - 1];
        uint32_t regionNumSolutions_ = result_.numSolutions_ - lastRegionNumSolutions_;
        const float averageHitCount = regionNumSolutions_ > 0 ? std::accumulate(result_.solutionHitCounts_ + lastRegionNumSolutions_, result_.solutionHitCounts_ + lastRegionNumSolutions_ + regionNumSolutions_, 0.0f) / static_cast<float>(regionNumSolutions_) : 0.0f;
        lastRegionNumSolutions_ = result_.numSolutions_;
        std::stringstream ss;
        ss << "Wedge id: " << region.id_
            << "\tsolutions: " << regionNumSolutions_
            << "\txAngleMin: " << region.xAngleMin_
            << "\taverage hit count: " << averageHitCount;
        LOG_DEBUG(ss.str());

        if (!regionNumSolutions_)
        {
            ss.str("");
            ss << "Wedge id: " << region.id_
                << "\tNo solutions found!";
            LOG_WARNING(ss.str());
            allRegionsContainHelix = false;
        }
    }
    EXPECT_TRUE(allRegionsContainHelix);
}
