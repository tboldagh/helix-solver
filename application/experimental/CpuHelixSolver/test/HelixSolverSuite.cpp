#include "CpuHelixSolver/HelixSolver.h"
#include "SplitterUsm/Splitter.h"
#include "CpuHelixSolver/Event.h"
#include "CpuHelixSolver/Result.h"
#include "CpuHelixSolver/Task.h"
#include "Logger/OstreamLogger.h"
#include "Logger/Logger.h"
#include "RootEventLoader/RootEventLoader.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

class HelixSolverTest : public ::testing::Test
{
protected:
    HelixSolverTest()
    : splitter_(getSplitterSettings())
    , helixSolver_(splitter_)
    , task_(event_, result_) {}

    ~HelixSolverTest() override = default;

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
        constexpr u_int8_t numXRanges = 8;
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

        // r: {335.897, 428.831, 521.728, 614.58, 707.379, 800.117, 892.785, 985.376, 1077.89, 1170.32};
        // phi: {0.562799, 0.5721, 0.5814, 0.5907, 0.600001, 0.609301, 0.618601, 0.627901, 0.637201, 0.646501};

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
    region_.pointListEnd_ = HelixSolver::SolutionHitsThreshold - 1;
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
    region_.pointListEnd_ = HelixSolver::SolutionHitsThreshold;
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
    region_.pointListEnd_ = HelixSolver::SolutionHitsThreshold;
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
    region_.pointListEnd_ = HelixSolver::SolutionHitsThreshold;
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
    region_.pointListEnd_ = HelixSolver::SolutionHitsThreshold;
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
        logger_.setMinSeverity(Logger::LogMessage::Severity::Info);
        Logger::ILogger::setGlobalInstance(&logger_);
    }
    
    ~SingleHelixDetectionTest() override = default;

    void createHelixPoints(float r, float phi, float xAngle, u_int8_t numPoints)
    {
        const float centerX = std::cos(phi) * r;
        const float centerY = std::sin(phi) * r;

        // Rotation range resulting in points distributed between the center and the edge of the detector in XY plane
        const float minRotation = std::atan(150 / r);
        const float maxRotation = std::atan(1000 / r);
        
        constexpr float zRangeMin = -100.0f;
        constexpr float zRangeMax = 3100.0f;

        // Note: z is not fully correct, but close enough for testing purposes
        for (u_int8_t i = 0; i < numPoints; ++i)
        {
            const float rotation = minRotation + i * (maxRotation - minRotation) / (numPoints - 1);
            std::pair<float, float> rotated = rotateXY(centerX, centerY, 0, 0, rotation);
            const u_int32_t index = event_.numPoints_++;
            event_.xs_[index] = rotated.first;
            event_.ys_[index] = rotated.second;
            event_.zs_[index] = (zRangeMin + i * (zRangeMax - zRangeMin) / (numPoints - 1)) * std::cos(xAngle);
        }
    }

    static std::pair<float, float> rotateXY(float centerX, float centerY, float x, float y, float angle)
    {
        const float xRotated = centerX + (x - centerX) * std::cos(angle) - (y - centerY) * std::sin(angle);
        const float yRotated = centerY + (x - centerX) * std::sin(angle) + (y - centerY) * std::cos(angle);
        return std::make_pair(xRotated, yRotated);
    }

    void createRunAndExtractResult(u_int16_t regionIndex, float r, float phi, float xAngle, u_int8_t numPoints)
    {
        // Create event
        createHelixPoints(r, phi, xAngle, numPoints);

        // for (u_int32_t i = 0; i < numPoints; ++i)
        // {
        //     std::cout << "\t(" << event_.xs_[i] << ", " << event_.ys_[i] << ", " << event_.zs_[i] << ")," << std::endl;
        // }

        regionSolverData_.regionId_ = regionIndex + 1;
        helixSolver_.solveRegion(task_, regionSolverData_);
    }

    static float rToQOverPt(float r)
    {
        return 1.0f / (r * HelixSolver::BMagnitude);
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
        std::unique_ptr<float[]> qOverPts{new float[result_.numSolutions_]};
        float* qOverPtsPtr = qOverPts.get();
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            qOverPtsPtr[i] = rToQOverPt(result_.solutionRs_[i]);
        }

        bool foundMatchingSolution = false;
        const float expectedQOverPt = 1.0f / (expectedR * HelixSolver::BMagnitude);
        const float qOverPtMin = expectedQOverPt - 0.05f * expectedQOverPt;
        const float qOverPtMax = expectedQOverPt + 0.05f * expectedQOverPt;
        const float phiMin = expectedPhi - 0.01f;
        const float phiMax = expectedPhi + 0.01f;
        for (u_int32_t i = 0; i < result_.numSolutions_; ++i)
        {
            foundMatchingSolution |= qOverPtsPtr[i] >= qOverPtMin && qOverPtsPtr[i] <= qOverPtMax && result_.solutionPhis_[i] >= phiMin && result_.solutionPhis_[i] <= phiMax;
        }

        if (!foundMatchingSolution)
        {
            LOG_WARNING("No matching solution found for r = " + std::to_string(expectedR) + ", phi = " + std::to_string(expectedPhi));
        }

        return foundMatchingSolution;
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

TEST_F(SingleHelixDetectionTest, BasicR2000Phi200XAngle04)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 2000.0f;
    constexpr float phi = 2.0f;
    constexpr u_int8_t numPoints = 12;
    constexpr float xAngle = 0.4f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);

    // saveResult("/tmp/ut_sandbox/result_BasicR2000Phi200XAngle04.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR2000Phi190XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 2000.0f;
    constexpr float phi = 1.9f;
    constexpr u_int8_t numPoints = 12;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR2000Phi190XAngle06.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR5000Phi210XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 5000.0f;
    constexpr float phi = 2.1f;
    constexpr u_int8_t numPoints = 10;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR5000Phi210XAngle06.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR10000Phi190XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 10000.0f;
    constexpr float phi = 1.9f;
    constexpr u_int8_t numPoints = 12;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR10000Phi180XAngle06.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, RotatedR2000Phi160XAngle04)
{
    // Define helix
    constexpr u_int16_t regionIndex = 0;    // Region requiring rotation due to atan2 discontinuity
    constexpr float r = 2000.0f;
    constexpr float phi = 1.6f;
    constexpr u_int8_t numPoints = 12;
    constexpr float xAngle = 0.4f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_RotatedR2000Phi160XAngle04.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, RotatedR2000Phi170XAngle05)
{
    // Define helix
    constexpr u_int16_t regionIndex = 0;    // Region requiring rotation due to atan2 discontinuity
    constexpr float r = 2000.0f;
    constexpr float phi = 1.7f;
    constexpr u_int8_t numPoints = 12;
    constexpr float xAngle = 0.5f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_RotatedR2000Phi170XAngle05.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, RotatedR24000Phi155XAngle05)
{
    // Define helix
    constexpr u_int16_t regionIndex = 0;    // Region requires rotation due to atan2 discontinuity
    constexpr float r = 24000.0f;
    constexpr float phi = 1.55f;    // Some points lay in negative y
    constexpr u_int8_t numPoints = 10;
    constexpr float xAngle = 0.4;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_RotatedR24000Phi150XAngle05.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

class MultipleHelixDetectionTest : public SingleHelixDetectionTest
{
protected:
    MultipleHelixDetectionTest() = default;
    ~MultipleHelixDetectionTest() override = default;

    class Helix
    {
    public:
        Helix(float r, float phi, float xAngle, u_int8_t numPoints)
        : r_(r), phi_(phi), xAngle_(xAngle), numPoints_(numPoints) {}

        float r_;
        float phi_;
        float xAngle_;
        u_int8_t numPoints_;
    };

    void createHelixPoints(const std::vector<Helix>& helixes)
    {
        for (const Helix& helix : helixes)
        {
            SingleHelixDetectionTest::createHelixPoints(helix.r_, helix.phi_, helix.xAngle_, helix.numPoints_);
        }
    }

    void createRunAndExtractResult(u_int16_t regionIndex, const std::vector<Helix>& helixes)
    {
        // Create event
        createHelixPoints(helixes);

        // for (u_int32_t i = 0; i < numPoints; ++i)
        // {
        //     std::cout << "\t(" << event_.xs_[i] << ", " << event_.ys_[i] << ", " << event_.zs_[i] << ")," << std::endl;
        // }

        regionSolverData_.regionId_ = regionIndex + 1;
        helixSolver_.solveRegion(task_, regionSolverData_);
    }
};

TEST_F(MultipleHelixDetectionTest, Basic)
{
    constexpr u_int16_t regionIndex = 1;
    const std::vector<Helix> helixes = {
        Helix(2000.0f, 1.9f, 0.8f, 12),
        Helix(2000.0f, 2.0f, 0.6f, 12),
        Helix(2000.0f, 2.1f, 0.5f, 12),
        Helix(5000.0f, 1.9f, 0.8f, 10),
        Helix(5000.0f, 2.0f, 0.4f, 10),
        Helix(5000.0f, 2.1f, 0.8f, 10),
        Helix(10000.0f, 1.9f, 0.8f, 12),
        Helix(10000.0f, 2.0f, 0.4f, 12),
        Helix(10000.0f, 2.1f, 0.1f, 8),
        Helix(20000.0f, 2.0f, 0.2f, 10),
        Helix(20000.0f, 2.1f, 0.7f, 10),
        Helix(20000.0f, 2.2f, 0.8f, 10)
    };

    createRunAndExtractResult(regionIndex, helixes);
    
    // saveResult("/tmp/ut_sandbox/result_MultipleHelixDetectionTest_Basic.csv");

    bool allHelixesFound = true;
    for (const Helix& helix : helixes)
    {
        allHelixesFound &= matchingSolutionExists(helix.r_, helix.phi_);
    }
    EXPECT_TRUE(allHelixesFound);
}

TEST_F(MultipleHelixDetectionTest, Rotated)
{
    constexpr u_int16_t regionIndex = 0;
    const std::vector<Helix> helixes = {
        Helix(2000.0f, 1.5f, 0.8f, 12),
        Helix(2000.0f, 1.6f, 0.5f, 12),
        Helix(2000.0f, 1.7f, 0.2f, 12),
        Helix(5000.0f, 1.5f, 0.8f, 10),
        Helix(5000.0f, 1.6f, 0.4f, 8),
        Helix(5000.0f, 1.7f, 0.9f, 10),
        Helix(10000.0f, 1.6f, 0.8f, 12),
        Helix(10000.0f, 1.6f, 0.4f, 12),
        Helix(10000.0f, 1.7f, 0.1f, 8),
        Helix(20000.0f, 1.6f, 0.2f, 10),
        Helix(20000.0f, 1.7f, 0.7f, 10),
        Helix(20000.0f, 1.8f, 0.8f, 12)
    };

    createRunAndExtractResult(regionIndex, helixes);
    
    // saveResult("/tmp/ut_sandbox/result_MultipleHelixDetectionTest_Rotated.csv");

    bool allHelixesFound = true;
    for (const Helix& helix : helixes)
    {
        allHelixesFound &= matchingSolutionExists(helix.r_, helix.phi_);
    }
    EXPECT_TRUE(allHelixesFound);
}

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
        }
        else if (region.xAngleMin_ < 0.8f || region.xAngleMin_ > 2.0f)
        {
            region.solutionHitsThreshold_ = 10;
            region.linesCrossingsThreshold_ = 3;
        }
        else if (region.xAngleMin_ < 1.0f || region.xAngleMin_ > 1.8f)
        {
            region.solutionHitsThreshold_ = 8;
            region.linesCrossingsThreshold_ = 3;
        }
        else if (region.xAngleMin_ < 1.4f || region.xAngleMin_ > 1.6f)
        {
            region.solutionHitsThreshold_ = 8;
            region.linesCrossingsThreshold_ = 3;
        }
        else
        {
            region.solutionHitsThreshold_ = 3;
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

    uint32_t lastRegionNumSolutions_ = 0;
    bool allRegionsContainHelix = true;
    for (u_int16_t regionId = 1; regionId <= splitter_.settings_.wedges_.getSize(); ++regionId)
    {
        regionSolverData_.regionId_ = regionId;
        helixSolver_.solveRegion(task_, regionSolverData_);

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
