#include "HelixSolverUsm/SingleRegionKernel.h"
#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "SplitterUsm/TestDataLoader.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <CL/sycl.hpp>
#include <numeric>


#include <fstream>

class SingleRegionKernelTest : public ::testing::Test
{
protected:
    SingleRegionKernelTest() = default;
    ~SingleRegionKernelTest() override = default;

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
     
    static void assertBeginEq(const u_int32_t* actual, const std::vector<u_int32_t>& expected, u_int32_t end)
    {
        constexpr u_int32_t begin = 0;
        assertRegionEq(actual, expected, begin, end);
    }
};

class RegionHitTest : public SingleRegionKernelTest
{
protected:
    RegionHitTest() = default;
    ~RegionHitTest() override = default;

    static constexpr float qOverPtMin = 1.0f;
    static constexpr float qOverPtMax = 2.0f;
    static constexpr float phi0Min = 1.0f;
    static constexpr float phi0Max = 2.0f;
};

TEST_F(RegionHitTest, AboveRegion)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 5.0f;

    EXPECT_FALSE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, TopRightCorner)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 4.0f - 1e-6f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, CrossingRight)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 3.5f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, BottomRightCorner)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 2.0f;
    constexpr float phi = 5.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, TopLeftAndBottomRightCorner)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 3.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, AboveLeftAndBelowRight)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 4.0f;
    constexpr float phi = 8.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, TopLeftCorner)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 2.0f;
    constexpr float phi = 4.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, CrossingLeft)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 2.5f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, CrossingLeftAndRight)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 0.5f;
    constexpr float phi = 2.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, BottomLeftCorner)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 2.0f;

    EXPECT_TRUE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

TEST_F(RegionHitTest, BelowRegion)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 1.0f;

    EXPECT_FALSE(SingleRegionKernel::regionHit(qOverPtMin, qOverPtMax, phi0Min, phi0Max, r, phi));
}

class FillNewPointListTest : public RegionHitTest
{
protected:
    FillNewPointListTest()
    {
        // Fill with dummy values make sure the results are not accidental
        for (u_int32_t i = 0; i < SingleRegionKernel::MaxPointListsPointsNum; ++i)
        {
            pointLists_[i] = 2137;
        }
    }

    ~FillNewPointListTest() override = default;

    static constexpr u_int32_t qOverPtDivisionLevel = 21;
    static constexpr u_int32_t phi0DivisionLevel = 37;
    SingleRegionKernel::AccumulatorRegion region_{qOverPtMin, qOverPtMax, phi0Min, phi0Max, qOverPtDivisionLevel, phi0DivisionLevel};
    
    float rs_[SingleRegionKernel::MaxPointsInRegion];
    float phis_[SingleRegionKernel::MaxPointsInRegion];
    u_int32_t pointLists_[SingleRegionKernel::MaxPointListsPointsNum];
};

TEST_F(FillNewPointListTest, SinglePointInRegion)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 3.0f;
    EXPECT_TRUE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, r, phi));

    rs_[0] = r;
    phis_[0] = phi;
    pointLists_[0] = 0;

    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 0;
    sourceRegion.pointListEnd = 1;
    region_.pointListBegin = sourceRegion.pointListEnd;
    region_.pointListEnd = sourceRegion.pointListEnd;

    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, sourceRegion.pointListEnd);
    EXPECT_EQ(region_.pointListEnd, 2);
    EXPECT_EQ(pointLists_[1], 0);
}

TEST_F(FillNewPointListTest, SinglePointNotInRegion)
{
    constexpr float r = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    constexpr float phi = 10.0f;
    EXPECT_FALSE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, r, phi));

    rs_[0] = r;
    phis_[0] = phi;
    pointLists_[0] = 0;

    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 0;
    sourceRegion.pointListEnd = 1;
    region_.pointListBegin = sourceRegion.pointListEnd;
    region_.pointListEnd = sourceRegion.pointListEnd;
    
    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, sourceRegion.pointListEnd);
    EXPECT_EQ(region_.pointListEnd, 1);
}

TEST_F(FillNewPointListTest, TwoPointsOneInRegion)
{
    rs_[0] = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;  
    phis_[0] = 10.0f;
    EXPECT_FALSE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, rs_[0], phis_[0]));

    rs_[1] = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    phis_[1] = 3.0f;
    EXPECT_TRUE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, rs_[1], phis_[1]));

    const std::vector<u_int32_t> sourcePointList = {0, 1};
    writeRegion(pointLists_, sourcePointList);
    
    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 0;
    sourceRegion.pointListEnd = 2;
    region_.pointListBegin = sourceRegion.pointListEnd;
    region_.pointListEnd = sourceRegion.pointListEnd;

    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, sourceRegion.pointListEnd);
    EXPECT_EQ(region_.pointListEnd, 3);
    assertBeginEq(pointLists_, sourcePointList, 2); // Assert no unwanted changes
    assertRegionEq(pointLists_, {1}, 2, 3);
}

class FillNewPointListMultiplePointsTest : public FillNewPointListTest
{
protected:
    FillNewPointListMultiplePointsTest()
    {
        for (float phi : inRegionPhis_)
        {
            EXPECT_TRUE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, r_, phi));
        }

        for (float phi : notInRegionPhis_)
        {
            EXPECT_FALSE(SingleRegionKernel::regionHit(region_.qOverPtMin, region_.qOverPtMax, region_.phi0Min, region_.phi0Max, r_, phi));
        }

        for (unsigned i = 0; i < inRegionIndexes_.size(); ++i)
        {
            rs_[inRegionIndexes_[i]] = r_;
            phis_[inRegionIndexes_[i]] = inRegionPhis_[i];
        }

        for (unsigned i = 0; i < notInRegionIndexes_.size(); ++i)
        {
            rs_[notInRegionIndexes_[i]] = r_;
            phis_[notInRegionIndexes_[i]] = notInRegionPhis_[i];
        }
    }

    ~FillNewPointListMultiplePointsTest() override = default;

    const float r_ = 2.0f / SingleRegionKernel::BMagnitude * 1.0f;
    const std::vector<float> inRegionPhis_ = { 2.5f, 2.75f, 3.0f, 3.25f, 3.5f};
    const std::vector<float> notInRegionPhis_ = { 0.0f, 1.0f, 5.0f, 6.0f, 7.0f};
    // Mixed on purpose
    const std::vector<u_int32_t> inRegionIndexes_ = {0, 3, 4, 8, 9};
    const std::vector<u_int32_t> notInRegionIndexes_ = {1, 2, 5, 6, 7};
};

TEST_F(FillNewPointListMultiplePointsTest, Basic)
{
    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(pointLists_, sourcePointList);

    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 0;
    sourceRegion.pointListEnd = 10;
    region_.pointListBegin = sourceRegion.pointListEnd;
    region_.pointListEnd = sourceRegion.pointListEnd;

    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, sourceRegion.pointListEnd);
    EXPECT_EQ(region_.pointListEnd, 15);
    assertBeginEq(pointLists_, sourcePointList, 10); // Assert no unwanted changes
    assertRegionEq(pointLists_, inRegionIndexes_, 10, 15);
}

TEST_F(FillNewPointListMultiplePointsTest, NewListNotJustAfterSource)
{
    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(pointLists_, sourcePointList);

    const std::vector<u_int32_t> unrelatedPointList = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    writeRegion(&pointLists_[10], unrelatedPointList);
    writeRegion(&pointLists_[20], unrelatedPointList);

    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 0;
    sourceRegion.pointListEnd = 10;
    region_.pointListBegin = 30;    // source + 2 unrelated
    region_.pointListEnd = 30;

    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, 30);
    EXPECT_EQ(region_.pointListEnd, 35);
    assertRegionEq(pointLists_, sourcePointList, 0, 10);  // Assert no unwanted changes
    assertRegionEq(pointLists_, unrelatedPointList, 10, 20);  // Assert no unwanted changes
    assertRegionEq(pointLists_, unrelatedPointList, 20, 30);  // Assert no unwanted changes
    assertRegionEq(pointLists_, inRegionIndexes_, 30, 35);
}

TEST_F(FillNewPointListMultiplePointsTest, SourceNotAtBegin)
{
    const std::vector<u_int32_t> unrelatedPointList = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    writeRegion(pointLists_, unrelatedPointList);

    const std::vector<u_int32_t> sourcePointList = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    writeRegion(&pointLists_[10], sourcePointList);

    writeRegion(&pointLists_[20], unrelatedPointList);

    SingleRegionKernel::AccumulatorRegion sourceRegion = region_;
    sourceRegion.pointListBegin = 10;
    sourceRegion.pointListEnd = 20;
    region_.pointListBegin = 30;    // unrelated + source + unrelated
    region_.pointListEnd = 30;

    SingleRegionKernel::fillNewPointList(region_, sourceRegion, pointLists_, rs_, phis_);
    EXPECT_EQ(region_.pointListBegin, 30);
    EXPECT_EQ(region_.pointListEnd, 35);
    assertRegionEq(pointLists_, unrelatedPointList, 0, 10);  // Assert no unwanted changes
    assertRegionEq(pointLists_, sourcePointList, 10, 20);  // Assert no unwanted changes
    assertRegionEq(pointLists_, unrelatedPointList, 20, 30);  // Assert no unwanted changes
    assertRegionEq(pointLists_, inRegionIndexes_, 30, 35);
}

class ProcessNextAccumulatorRegionTest : public FillNewPointListTest
{
protected:
    ProcessNextAccumulatorRegionTest() = default;
    ~ProcessNextAccumulatorRegionTest() override = default;

    const u_int16_t regionId_ = 42;
    SingleRegionKernel::AccumulatorRegion accumulatorRegions_[SingleRegionKernel::MaxAccumulatorRegionStackSize];
    u_int8_t accumulatorRegionStackSize_ = 0;
    u_int32_t indexes_[SingleRegionKernel::MaxPointsInRegion];
    u_int32_t regionNumSolutions_[ResultUsm::MaxRegions];
    u_int8_t solutionHitCounts_[ResultUsm::MaxSolutions];
    float solutionRs_[ResultUsm::MaxSolutions];
    float solutionPhis_[ResultUsm::MaxSolutions];
};

TEST_F(ProcessNextAccumulatorRegionTest, DropRegionIfNumberOfPointsIsBelowThreshold)
{
    region_.qOverPtDivisionLevel = SingleRegionKernel::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel = SingleRegionKernel::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin = 0;
    region_.pointListEnd = SingleRegionKernel::SolutionHitsThreshold - 1;
    accumulatorRegions_[0] = region_;
    accumulatorRegionStackSize_ = 1;

    SingleRegionKernel::processNextAccumulatorRegion(regionId_, accumulatorRegions_, accumulatorRegionStackSize_, pointLists_, indexes_, rs_, phis_, regionNumSolutions_, solutionHitCounts_, solutionRs_, solutionPhis_);
    EXPECT_EQ(accumulatorRegionStackSize_, 0);
}

TEST_F(ProcessNextAccumulatorRegionTest, DivideInBothDimensionsIfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel = SingleRegionKernel::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel = SingleRegionKernel::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin = 0;
    region_.pointListEnd = SingleRegionKernel::SolutionHitsThreshold;
    accumulatorRegions_[0] = region_;
    accumulatorRegionStackSize_ = 1;

    SingleRegionKernel::processNextAccumulatorRegion(regionId_, accumulatorRegions_, accumulatorRegionStackSize_, pointLists_, indexes_, rs_, phis_, regionNumSolutions_, solutionHitCounts_, solutionRs_, solutionPhis_);
    EXPECT_EQ(accumulatorRegionStackSize_, 4);

    std::vector<SingleRegionKernel::AccumulatorRegion> actualRegions(accumulatorRegions_, accumulatorRegions_ + accumulatorRegionStackSize_);
    const float qOverPtMiddle = (qOverPtMin + qOverPtMax) / 2;
    const float phi0Middle = (phi0Min + phi0Max) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Min, phi0Middle, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Min, phi0Middle, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Middle, phi0Max, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Middle, phi0Max, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel + 1)));
}

TEST_F(ProcessNextAccumulatorRegionTest, DividePhi0IfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel = SingleRegionKernel::QOverPtMaxDivisionLevel;
    region_.phi0DivisionLevel = SingleRegionKernel::Phi0MaxDivisionLevel - 1;
    region_.pointListBegin = 0;
    region_.pointListEnd = SingleRegionKernel::SolutionHitsThreshold;
    accumulatorRegions_[0] = region_;
    accumulatorRegionStackSize_ = 1;

    SingleRegionKernel::processNextAccumulatorRegion(regionId_, accumulatorRegions_, accumulatorRegionStackSize_, pointLists_, indexes_, rs_, phis_, regionNumSolutions_, solutionHitCounts_, solutionRs_, solutionPhis_);
    EXPECT_EQ(accumulatorRegionStackSize_, 2);

    std::vector<SingleRegionKernel::AccumulatorRegion> actualRegions(accumulatorRegions_, accumulatorRegions_ + accumulatorRegionStackSize_);
    const float phi0Middle = (phi0Min + phi0Max) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMin, qOverPtMax, phi0Min, phi0Middle, region_.qOverPtDivisionLevel, region_.phi0DivisionLevel + 1)));
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMin, qOverPtMax, phi0Middle, phi0Max, region_.qOverPtDivisionLevel, region_.phi0DivisionLevel + 1)));
}

TEST_F(ProcessNextAccumulatorRegionTest, DivideQOverPtIfMaxDivisionLevelsNotReached)
{
    region_.qOverPtDivisionLevel = SingleRegionKernel::QOverPtMaxDivisionLevel - 1;
    region_.phi0DivisionLevel = SingleRegionKernel::Phi0MaxDivisionLevel;
    region_.pointListBegin = 0;
    region_.pointListEnd = SingleRegionKernel::SolutionHitsThreshold;
    accumulatorRegions_[0] = region_;
    accumulatorRegionStackSize_ = 1;

    SingleRegionKernel::processNextAccumulatorRegion(regionId_, accumulatorRegions_, accumulatorRegionStackSize_, pointLists_, indexes_, rs_, phis_, regionNumSolutions_, solutionHitCounts_, solutionRs_, solutionPhis_);
    EXPECT_EQ(accumulatorRegionStackSize_, 2);

    std::vector<SingleRegionKernel::AccumulatorRegion> actualRegions(accumulatorRegions_, accumulatorRegions_ + accumulatorRegionStackSize_);
    const float qOverPtMiddle = (qOverPtMin + qOverPtMax) / 2;
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMin, qOverPtMiddle, phi0Min, phi0Max, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel)));
    EXPECT_THAT(actualRegions, ::testing::Contains(SingleRegionKernel::AccumulatorRegion(qOverPtMiddle, qOverPtMax, phi0Min, phi0Max, region_.qOverPtDivisionLevel + 1, region_.phi0DivisionLevel)));
}

TEST_F(ProcessNextAccumulatorRegionTest, AddSolutionIfMaxDivisionLevelsReached)
{
    region_.qOverPtDivisionLevel = SingleRegionKernel::QOverPtMaxDivisionLevel;
    region_.phi0DivisionLevel = SingleRegionKernel::Phi0MaxDivisionLevel;
    region_.pointListBegin = 0;
    region_.pointListEnd = SingleRegionKernel::SolutionHitsThreshold;
    accumulatorRegions_[0] = region_;
    accumulatorRegionStackSize_ = 1;

    const u_int16_t regionIndex = regionId_ - 1;
    const u_int32_t solutionIndex = regionIndex * ResultUsm::MaxSolutionsPerRegion;

    SingleRegionKernel::processNextAccumulatorRegion(regionId_, accumulatorRegions_, accumulatorRegionStackSize_, pointLists_, indexes_, rs_, phis_, regionNumSolutions_, solutionHitCounts_, solutionRs_, solutionPhis_);
    EXPECT_EQ(accumulatorRegionStackSize_, 0);
    EXPECT_EQ(regionNumSolutions_[regionIndex], 1);
    EXPECT_EQ(solutionHitCounts_[solutionIndex], SingleRegionKernel::SolutionHitsThreshold);
    const float expectedR = 1 / (0.5f * (qOverPtMin + qOverPtMax) * SingleRegionKernel::BMagnitude);
    EXPECT_FLOAT_EQ(solutionRs_[solutionIndex], expectedR);
    const float expectedPhi = SingleRegionKernel::wrapMinusPiToPi(0.5f * (phi0Min + phi0Max) + 0.5f * M_PI);
    EXPECT_FLOAT_EQ(solutionPhis_[solutionIndex], expectedPhi);
}

class SingleHelixDetectionTest : public ::testing::Test
{
protected:
    SingleHelixDetectionTest()
    : settings_(getSplitterSettings()),
        splitter_(settings_) {}
    ~SingleHelixDetectionTest() override = default;

    static SplitterSettings getSplitterSettings()
    {
        constexpr float maxAbsXy = 1100.0;
        constexpr float maxAbsZ = 3100.0;
        constexpr float minZAngle = 0.0;
        constexpr float maxZAngle = 2.0 * M_PI;
        constexpr float minXAgle = 1.0 / 16 * M_PI;
        constexpr float maxXAgle = 15.0 / 16 * M_PI;
        constexpr float poleRegionAngle = 1.0 / 16 * M_PI;
        constexpr float interactionRegionMin = -250.0;
        constexpr float interactionRegionMax = 250.0;
        constexpr float zAngleMargin = 4.0 / 256 * M_PI;
        constexpr float xAngleMargin = 2.0 / 256 * M_PI;
        constexpr u_int8_t numZRanges = 16;
        constexpr u_int8_t numXRanges = 8;
        return SplitterSettings(
            maxAbsXy, maxAbsZ,
            minZAngle, maxZAngle,
            minXAgle, maxXAgle,
            poleRegionAngle,
            interactionRegionMin, interactionRegionMax,
            zAngleMargin, xAngleMargin,
            numZRanges, numXRanges
        );
    }

    void createHelixPoints(float r, float phi, float xAngle, u_int8_t numPoints, float* xs, float* ys, float* zs, u_int32_t startIndex = 0)
    {
        const float centerX = std::cos(phi) * r;
        const float centerY = std::sin(phi) * r;

        // Rotation range resulting in points distributed between the center and the edge of the detector in XY plane
        const float minRotation = std::atan(10 / r);
        const float maxRotation = std::atan(1000 / r);
        
        constexpr float zRangeMin = -100.0f;
        constexpr float zRangeMax = 3100.0f;

        // Note: z is not fully correct, but close enough for testing purposes
        for (u_int8_t i = 0; i < numPoints; ++i)
        {
            const float rotation = minRotation + i * (maxRotation - minRotation) / (numPoints - 1);
            std::pair<float, float> rotated = rotateXY(centerX, centerY, 0, 0, rotation);
            xs[i + startIndex] = rotated.first;
            ys[i + startIndex] = rotated.second;
            zs[i + startIndex] = (zRangeMin + i * (zRangeMax - zRangeMin) / (numPoints - 1)) * std::cos(xAngle);
        }
    }

    static std::pair<float, float> rotateXY(float centerX, float centerY, float x, float y, float angle)
    {
        const float xRotated = centerX + (x - centerX) * std::cos(angle) - (y - centerY) * std::sin(angle);
        const float yRotated = centerY + (x - centerX) * std::sin(angle) + (y - centerY) * std::cos(angle);
        return std::make_pair(xRotated, yRotated);
    }

    static float rToQOverPt(float r)
    {
        return 1.0f / (r * SingleRegionKernel::BMagnitude);
    }

    void saveResult(const std::string& path)
    {
        std::ofstream resultFile(path);
        for (u_int32_t i = 0; i < regionNumSolutions_; ++i)
        {
            resultFile << i << ",\t" 
                    << static_cast<unsigned>(hitCounts_[i]) << ",\t"
                    << rs_[i] << ",\t"
                    << phis_[i] << ",\t"
                    << qOverPts_[i] << std::endl;
        }
        resultFile.close();
    }

    void extractResult(u_int16_t regionIndex)
    {
        rs_.clear();
        phis_.clear();
        hitCounts_.clear();
        qOverPts_.clear();

        regionNumSolutions_ = deviceRegionNumSolutions_[regionIndex];
        const u_int32_t regionSolutionsBegin = ResultUsm::MaxSolutionsPerRegion * regionIndex;
        for (u_int32_t i = 0; i < regionNumSolutions_; ++i)
        {
            rs_.push_back(deviceSolutionRs_[regionSolutionsBegin + i]);
            phis_.push_back(deviceSolutionPhis_[regionSolutionsBegin + i]);
            hitCounts_.push_back(deviceSolutionHitCounts_[regionSolutionsBegin + i]);
        }

        qOverPts_.resize(regionNumSolutions_);
        std::transform(rs_.begin(), rs_.end(), qOverPts_.begin(), rToQOverPt);
    }

    void copyEventToDevice()
    {
        *deviceNumPoints_ = event_.hostNumPoints_;
        for (u_int32_t i = 0; i < event_.hostNumPoints_; ++i)
        {
            deviceXs_[i] = event_.hostXs_[i];
            deviceYs_[i] = event_.hostYs_[i];
            deviceZs_[i] = event_.hostZs_[i];
            deviceLayers_[i] = event_.hostLayers_[i];
        }
    }

    void initKernel(SingleRegionKernel& kernel)
    {
        kernel.deviceNumPoints_ = deviceNumPoints_.get();
        kernel.deviceXs_ = deviceXs_.get();
        kernel.deviceYs_ = deviceYs_.get();
        kernel.deviceZs_ = deviceZs_.get();
        kernel.deviceLayers_ = deviceLayers_.get();
        kernel.deviceNumSolutions_ = deviceNumSolutions_.get();
        kernel.deviceRegionNumSolutions_ = deviceRegionNumSolutions_.get();
        kernel.deviceSolutionHitCounts_ = deviceSolutionHitCounts_.get();
        kernel.deviceSolutionRs_ = deviceSolutionRs_.get();
        kernel.deviceSolutionPhis_ = deviceSolutionPhis_.get();
    }

    void createRunAndExtractResult(u_int16_t regionIndex, float r, float phi, float xAngle, u_int8_t numPoints)
    {
        // Create event
        event_.hostNumPoints_ = numPoints;
        createHelixPoints(r, phi, xAngle, numPoints, event_.hostXs_, event_.hostYs_, event_.hostZs_);

        // for (u_int32_t i = 0; i < numPoints; ++i)
        // {
        //     std::cout << "\t(" << event_.hostXs_[i] << ", " << event_.hostYs_[i] << ", " << event_.hostZs_[i] << ")," << std::endl;
        // }

        // SplitterSettings::Wedge region = settings_.wedges_[regionIndex];
        // std::cout << region.zAngleMin_ << " " << region.zAngleMax_ << " " << region.xAngleMin_ << " " << region.xAngleMax_ << std::endl;

        // Copy event data to faked device memory
        copyEventToDevice();

        // Run kernel
        SingleRegionKernel kernel(&splitter_, &event_, &result_);
        initKernel(kernel);
        kernel(sycl::id<1>(regionIndex));

        extractResult(regionIndex);
    }

    bool matchingSolutionExists(float expectedR, float expectedPhi)
    {
        bool foundMatchingSolution = false;
        const float expectedQOverPt = 1.0f / (expectedR * SingleRegionKernel::BMagnitude);
        const float qOverPtMin = expectedQOverPt - 0.05f * expectedQOverPt;
        const float qOverPtMax = expectedQOverPt + 0.05f * expectedQOverPt;
        const float phiMin = expectedPhi - 0.01f;
        const float phiMax = expectedPhi + 0.01f;
        for (u_int32_t i = 0; i < regionNumSolutions_; ++i)
        {
            foundMatchingSolution |= qOverPts_[i] >= qOverPtMin && qOverPts_[i] <= qOverPtMax && phis_[i] >= phiMin && phis_[i] <= phiMax;
        }

        if (!foundMatchingSolution)
        {
            LOG_WARNING("No matching solution found for r = " + std::to_string(expectedR) + ", phi = " + std::to_string(expectedPhi));
        }

        return foundMatchingSolution;
    }

    void assertSolutionsCorrect(float expectedR, float expectedPhi, u_int8_t numPoints)
    {
        for (u_int32_t i = 0; i < regionNumSolutions_; ++i)
        {
            EXPECT_LE(hitCounts_[i], numPoints);
        }
        EXPECT_TRUE(matchingSolutionExists(expectedR, expectedPhi));
    }

    static const std::string TestDataDir;
    static constexpr EventUsm::EventId eventId = 42;
    static constexpr ResultUsm::ResultId resultId = 42;
    SplitterSettings settings_;
    Splitter splitter_;
    EventUsm event_{eventId};
    ResultUsm result_{resultId};

    // Fake device allocated memory
    std::unique_ptr<u_int32_t> deviceNumPoints_ = std::make_unique<u_int32_t>();
    std::unique_ptr<float[]> deviceXs_{new float[EventUsm::MaxPoints]};
    std::unique_ptr<float[]> deviceYs_{new float[EventUsm::MaxPoints]};
    std::unique_ptr<float[]> deviceZs_{new float[EventUsm::MaxPoints]};
    std::unique_ptr<EventUsm::LayerNumber[]> deviceLayers_{new EventUsm::LayerNumber[EventUsm::MaxPoints]};
    std::unique_ptr<u_int32_t> deviceNumSolutions_ = std::make_unique<u_int32_t>();
    std::unique_ptr<u_int32_t[]> deviceRegionNumSolutions_{new u_int32_t[ResultUsm::MaxRegions]};
    std::unique_ptr<u_int8_t[]> deviceSolutionHitCounts_{new u_int8_t[ResultUsm::MaxSolutions]};
    std::unique_ptr<float[]> deviceSolutionRs_{new float[ResultUsm::MaxSolutions]};
    std::unique_ptr<float[]> deviceSolutionPhis_{new float[ResultUsm::MaxSolutions]};

    // Extracted result
    u_int32_t regionNumSolutions_; 
    std::vector<float> rs_;
    std::vector<float> phis_;
    std::vector<u_int8_t> hitCounts_;
    std::vector<float> qOverPts_;
};
const std::string SingleHelixDetectionTest::TestDataDir = "/helix/repo/application/experimental/SplitterUsm/test-data";

TEST_F(SingleHelixDetectionTest, BasicR1050Phi200XAngle04)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 1050.0f;
    constexpr float phi = 2.0f;
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.4f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR1050Phi200XAngle04.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR2000Phi190XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 2000.0f;
    constexpr float phi = 1.9f;
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR2000Phi175XAngle04.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR5000Phi210XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 5000.0f;
    constexpr float phi = 2.1f;
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR5000Phi210XAngle06.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, BasicR10000Phi180XAngle06)
{
    // Define helix
    constexpr u_int16_t regionIndex = 1;
    constexpr float r = 10000.0f;
    constexpr float phi = 1.9f;
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.6f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_BasicR10000Phi180XAngle06.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, RotatedR1200Phi160XAngle04)
{
    // Define helix
    constexpr u_int16_t regionIndex = 0;    // Region requiring rotation due to atan2 discontinuity
    constexpr float r = 1200.0f;
    constexpr float phi = 1.6f;
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.4f;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_RotatedR1200Phi160XAngle04.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

TEST_F(SingleHelixDetectionTest, RotatedR2000Phi170XAngle05)
{
    // Define helix
    constexpr u_int16_t regionIndex = 0;    // Region requiring rotation due to atan2 discontinuity
    constexpr float r = 2000.0f;
    constexpr float phi = 1.7f;
    constexpr u_int8_t numPoints = 8;
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
    constexpr u_int8_t numPoints = 8;
    constexpr float xAngle = 0.4;

    createRunAndExtractResult(regionIndex, r, phi, xAngle, numPoints);
    
    // saveResult("/tmp/ut_sandbox/result_RotatedR24000Phi150XAngle05.csv");

    assertSolutionsCorrect(r, phi, numPoints);
}

class MultipleHelixDetectionTest : public SingleHelixDetectionTest
{
protected:
    MultipleHelixDetectionTest()
    : logger_(std::cout)
    {
        logger_.setMinSeverity(Logger::LogMessage::Severity::Info);
        Logger::ILogger::setGlobalInstance(&logger_);
    }

    ~MultipleHelixDetectionTest()
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

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

    void createHelixPoints(const Helix& helix, float* xs, float* ys, float* zs, u_int32_t startIndex = 0)
    {
        SingleHelixDetectionTest::createHelixPoints(helix.r_, helix.phi_, helix.xAngle_, helix.numPoints_, xs, ys, zs, startIndex);
    }

    void createHelixPoints(const std::vector<Helix>& helixes, float* xs, float* ys, float* zs, u_int32_t startIndex = 0)
    {
        for (const Helix& helix : helixes)
        {
            createHelixPoints(helix, xs, ys, zs, startIndex);
            startIndex += helix.numPoints_;
        }
    }

    void createRunAndExtractResult(u_int16_t regionIndex, const std::vector<Helix>& helixes)
    {
        // Create event
        event_.hostNumPoints_ = std::accumulate(helixes.begin(), helixes.end(), 0, [](u_int32_t sum, const Helix& helix) { return sum + helix.numPoints_; });
        createHelixPoints(helixes, event_.hostXs_, event_.hostYs_, event_.hostZs_);

        for (u_int32_t i = 0; i < event_.hostNumPoints_; ++i)
        {
            std::stringstream ss;
            ss << "\t(" << event_.hostXs_[i] << ", " << event_.hostYs_[i] << ", " << event_.hostZs_[i] << "),";
            LOG_DEBUG(ss);
        }

        SplitterSettings::Wedge region = settings_.wedges_[regionIndex];
        std::stringstream ss;
        ss << "wedge: " << region.zAngleMin_ << " " << region.zAngleMax_ << " " << region.xAngleMin_ << " " << region.xAngleMax_;
        LOG_DEBUG(ss.str());

        // Copy event data to faked device memory
        copyEventToDevice();

        // Run kernel
        SingleRegionKernel kernel(&splitter_, &event_, &result_);
        initKernel(kernel);
        kernel(sycl::id<1>(regionIndex));

        extractResult(regionIndex);
    }

    Logger::OstreamLogger logger_;
};

TEST_F(MultipleHelixDetectionTest, Basic)
{
    // Define helixes
    constexpr u_int16_t regionIndex = 1;
    const std::vector<Helix> helixes = {
        Helix(1200.0f, 2.0f, 0.1f, 8),
        Helix(1200.0f, 2.1f, 0.4f, 10),
        Helix(2000.0f, 1.9f, 0.8f, 10),
        Helix(2000.0f, 2.0f, 0.5f, 12),
        Helix(2000.0f, 2.1f, 0.2f, 8),
        Helix(5000.0f, 1.9f, 0.8f, 8),
        Helix(5000.0f, 2.0f, 0.4f, 8),
        Helix(5000.0f, 2.1f, 0.9f, 10),
        Helix(10000.0f, 1.9f, 0.8f, 10),
        Helix(10000.0f, 2.0f, 0.4f, 12),
        Helix(10000.0f, 2.1f, 0.1f, 8),
        Helix(20000.0f, 2.0f, 0.2f, 8),
        Helix(20000.0f, 2.1f, 0.7f, 8),
        Helix(20000.0f, 2.2f, 1.0f, 8)
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
    // Define helixes
    constexpr u_int16_t regionIndex = 0;
    const std::vector<Helix> helixes = {
        Helix(1200.0f, 1.5f, 0.1f, 8),
        Helix(1200.0f, 1.8f, 0.4f, 10),
        Helix(2000.0f, 1.5f, 0.8f, 10),
        Helix(2000.0f, 1.6f, 0.5f, 12),
        Helix(2000.0f, 1.7f, 0.2f, 8),
        Helix(5000.0f, 1.5f, 0.8f, 8),
        Helix(5000.0f, 1.6f, 0.4f, 8),
        Helix(5000.0f, 1.7f, 0.9f, 10),
        Helix(10000.0f, 1.5f, 0.8f, 10),
        Helix(10000.0f, 1.6f, 0.4f, 12),
        Helix(10000.0f, 1.7f, 0.1f, 8),
        Helix(20000.0f, 1.6f, 0.2f, 8),
        Helix(20000.0f, 1.7f, 0.7f, 8),
        Helix(20000.0f, 1.8f, 1.0f, 8)
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
    // Goal is to assert that the kernel is able to find helixes in all regions.

    const std::string eventPath = TestDataDir + "/event_0.csv";
    constexpr EventUsm::EventId eventId = 42;
    std::optional<std::unique_ptr<EventUsm>> eventOptional = TestDataLoader::readEvent(eventPath, eventId);
    ASSERT_TRUE(eventOptional.has_value());
    const auto& event = *eventOptional->get();
    event_.hostNumPoints_ = event.hostNumPoints_;
    for (u_int32_t i = 0; i < event.hostNumPoints_; ++i)
    {
        event_.hostXs_[i] = event.hostXs_[i];
        event_.hostYs_[i] = event.hostYs_[i];
        event_.hostZs_[i] = event.hostZs_[i];
        event_.hostLayers_[i] = event.hostLayers_[i];
    }

    for (u_int32_t i = 0; i < event_.hostNumPoints_; ++i)
    {
        std::stringstream ss;
        ss << "\t(" << event_.hostXs_[i] << ", " << event_.hostYs_[i] << ", " << event_.hostZs_[i] << "),";
        LOG_DEBUG(ss);
    }

    copyEventToDevice();

    // logger_.setMinSeverity(Logger::LogMessage::Severity::Debug);

    SingleRegionKernel kernel(&splitter_, &event_, &result_);
    initKernel(kernel);
    bool allRegionsContainHelix = true;
    for (u_int16_t regionIndex = 0; regionIndex < settings_.wedges_.getSize(); ++regionIndex)
    {
        kernel(sycl::id<1>(regionIndex));

        extractResult(regionIndex);

        SplitterSettings::Wedge region = settings_.wedges_[regionIndex];
        std::stringstream ss;
        ss << "Wedge id: " << region.id_ << "\tnumSolutions: " << regionNumSolutions_;
        LOG_DEBUG(ss.str());

        if (!regionNumSolutions_)
        {
            ss << "\tNo solutions found!";
            LOG_WARNING(ss.str());
            allRegionsContainHelix = false;
        }
    }
    EXPECT_TRUE(allRegionsContainHelix);
}




// TEST_F(SingleHelixDetectionTest, Basic2)
// {
//     // Read event
//     const std::string eventPath = TestDataDir + "/event_0.csv";
//     constexpr EventUsm::EventId eventId = 42;
//     std::optional<std::unique_ptr<EventUsm>> eventOptional = TestDataLoader::readEvent(eventPath, eventId);
//     ASSERT_TRUE(eventOptional.has_value());
//     const auto& event = *eventOptional->get();

//     // Create Splitter
//     SplitterSettings settings = getSplitterSettings();
//     Splitter splitter(settings);

//     // Select one region to test
//     constexpr u_int16_t regionId = 2;
//     constexpr u_int16_t regionIndex = 1;
//     SplitterSettings::Wedge region = settings.wedges_[regionIndex];
//     std::cout << region.zAngleMin_ << " " << region.zAngleMax_ << " " << region.xAngleMin_ << " " << region.xAngleMax_ << std::endl;

//     // Create result
//     constexpr ResultUsm::ResultId resultId = 42;
//     std::unique_ptr<ResultUsm> result = std::make_unique<ResultUsm>(resultId);

//     // Fake device allocated memory
//     std::unique_ptr<u_int32_t> deviceNumPoints = std::make_unique<u_int32_t>();
//     std::unique_ptr<float[]> deviceXs(new float[EventUsm::MaxPoints]);
//     std::unique_ptr<float[]> deviceYs(new float[EventUsm::MaxPoints]);
//     std::unique_ptr<float[]> deviceZs(new float[EventUsm::MaxPoints]);
//     std::unique_ptr<EventUsm::LayerNumber[]> deviceLayers(new EventUsm::LayerNumber[EventUsm::MaxPoints]);
//     std::unique_ptr<u_int32_t> deviceNumSolutions = std::make_unique<u_int32_t>();
//     std::unique_ptr<u_int32_t[]> deviceRegionNumSolutions(new u_int32_t[ResultUsm::MaxRegions]);
//     std::unique_ptr<u_int8_t[]> deviceSolutionHitCounts(new u_int8_t[ResultUsm::MaxSolutions]);
//     std::unique_ptr<float[]> deviceSolutionRs(new float[ResultUsm::MaxSolutions]);
//     std::unique_ptr<float[]> deviceSolutionPhis(new float[ResultUsm::MaxSolutions]);

//     // Copy event data to faked device memory
//     *deviceNumPoints = event.hostNumPoints_;
//     for (u_int32_t i = 0; i < event.hostNumPoints_; ++i)
//     {
//         deviceXs[i] = event.hostXs_[i];
//         deviceYs[i] = event.hostYs_[i];
//         deviceZs[i] = event.hostZs_[i];
//         deviceLayers[i] = event.hostLayers_[i];
//     }

//     // Create kernel
//     SingleRegionKernel kernel(&splitter, &event, result.get());
//     kernel.deviceNumPoints_ = deviceNumPoints.get();
//     kernel.deviceXs_ = deviceXs.get();
//     kernel.deviceYs_ = deviceYs.get();
//     kernel.deviceZs_ = deviceZs.get();
//     kernel.deviceLayers_ = deviceLayers.get();
//     kernel.deviceNumSolutions_ = deviceNumSolutions.get();
//     kernel.deviceRegionNumSolutions_ = deviceRegionNumSolutions.get();
//     kernel.deviceSolutionHitCounts_ = deviceSolutionHitCounts.get();
//     kernel.deviceSolutionRs_ = deviceSolutionRs.get();
//     kernel.deviceSolutionPhis_ = deviceSolutionPhis.get();

//     // Run kernel
//     kernel(sycl::id<1>(regionIndex));

//     // Save results to file
//     const std::string sandboxPath = "/tmp/ut_sandbox";
//     const std::string resultPath = sandboxPath + "/result_0.csv";
//     std::ofstream resultFile(resultPath);
//     const u_int32_t regionNumSolutions = deviceRegionNumSolutions[regionIndex];
//     const u_int32_t regionSolutionsBegin = ResultUsm::MaxSolutionsPerRegion * regionIndex;
//     std::cout << regionNumSolutions << std::endl;
//     for (u_int32_t i = 0; i < regionNumSolutions; ++i)
//     {
//         resultFile << i << ",\t" 
//                 << static_cast<unsigned>(deviceSolutionHitCounts[regionSolutionsBegin + i]) << ",\t"
//                 << deviceSolutionRs[regionSolutionsBegin + i] << ",\t"
//                 << deviceSolutionPhis[regionSolutionsBegin + i] << "\n";
//     }

//     resultFile.close();
    


//     // constexpr float bMagnitude = 3.8f;
//     // constexpr float qOverPt = 0.5f;
//     // constexpr float phi0 = 0.0f;
//     // constexpr float r = 1 / (qOverPt * bMagnitude);
//     // constexpr float phi = phi0 + 0.5f * M_PI;

//     // const std::vector<float> rs = {r};
//     // const std::vector<float> phis = {phi};
//     // const std::vector<u_int32_t> indexes = {0};

//     // const std::vector<u_int32_t> regionIds = {0};
//     // const std::vector<u_int32_t> regionNumSolutions = {0};
//     // const std::vector<u_int8_t> solutionHitCounts = {1};
//     // const std::vector<float> solutionRs = {r};
//     // const std::vector<float> solutionPhis = {phi};

//     // const std::vector<u_int32_t> expectedRegionIds = {0};
//     // const std::vector<u_int32_t> expectedRegionNumSolutions = {1};
//     // const std::vector<u_int8_t> expectedSolutionHitCounts = {1};
//     // const std::vector<float> expectedSolutionRs = {r};
//     // const std::vector<float> expectedSolutionPhis = {phi};

//     // SingleRegionKernel::SingleHelixDetectionResult result;
//     // result.regionIds = regionIds.data();
//     // result.regionNumSolutions = regionNumSolutions.data();
//     // result.solutionHitCounts = solutionHitCounts.data();
//     // result.solutionRs = solutionRs.data();
//     // result.solutionPhis = solutionPhis.data();

//     // SingleRegionKernel::singleHelixDetection(rs.data(), phis.data(), indexes.data(), rs.size(), result);

//     // EXPECT_EQ(result.regionIds, expectedRegionIds.data());
//     // EXPECT_EQ(result.regionNumSolutions, expectedRegionNumSolutions.data());
//     // EXPECT_EQ(result.solutionHitCounts, expectedSolutionHitCounts.data());
//     // EXPECT_EQ(result.solutionRs, expectedSolutionRs.data());
//     // EXPECT_EQ(result.solutionPhis, expectedSolutionPhis.data());
// }