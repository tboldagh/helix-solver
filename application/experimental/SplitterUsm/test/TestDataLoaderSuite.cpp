#include "SplitterUsm/TestDataLoader.h"

#include <gtest/gtest.h>
#include <fstream>

class TestDataLoaderTest : public ::testing::Test
{
protected:
    TestDataLoaderTest()
    : splitterSettings_(getSplitterSettings()) {}

    ~TestDataLoaderTest() override {}

    static SplitterSettings getSplitterSettings()
    {
        constexpr float maxAbsXy = 1100.0;
        constexpr float maxAbsZ = 3100.0;
        constexpr float minZAngle = 0.0;
        constexpr float maxZAngle = 2.0 * M_PI;
        constexpr float minXAgle = 1.0 / 16 * M_PI;
        constexpr float maxXAgle = 15.0 / 16 * M_PI;
        constexpr float poleRegionAngle = 1.0 / 16 * M_PI;
        constexpr float interactionRegionMin = -400.0;
        constexpr float interactionRegionMax = 400.0;
        constexpr float zAngleMargin = 2.0 / 256 * M_PI;
        constexpr float xAngleMargin = 1.0 / 256 * M_PI;
        constexpr u_int8_t numZRanges = 8;
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

    const std::string sandboxPath_ = "/tmp/ut_sandbox";
    const SplitterSettings splitterSettings_;
};

TEST_F(TestDataLoaderTest, WriteSplitterSettings)
{
    ASSERT_TRUE(splitterSettings_.isValid());

    const std::string path = sandboxPath_ + "/SplitterSettings.json";
    ASSERT_TRUE(TestDataLoader::writeSplitterSettings(path, splitterSettings_));

    // Remove the file
    ASSERT_EQ(std::remove(path.c_str()), 0);
}

TEST_F(TestDataLoaderTest, WriteAndReadSplitterSettings)
{
    ASSERT_TRUE(splitterSettings_.isValid());

    const std::string path = sandboxPath_ + "/SplitterSettings.json";
    ASSERT_TRUE(TestDataLoader::writeSplitterSettings(path, splitterSettings_));

    std::optional<SplitterSettings> readSettings = TestDataLoader::readSplitterSettings(path);
    ASSERT_TRUE(readSettings.has_value());
    ASSERT_EQ(splitterSettings_, readSettings.value());

    // Remove the file
    ASSERT_EQ(std::remove(path.c_str()), 0);
}

TEST_F(TestDataLoaderTest, ReadPointsWithRegionIds)
{
    const std::string path = sandboxPath_ + "/points_with_region_ids.csv";
    const std::string content = R"(x, y, z, region_ids
            -59.243309, 115.000717, -1123.19995, 2
            154.974457, 28.9229641, -1524.40002, 1
            -67.6243515, 131.420441, -1123.19995, 21, 37
            -85.303093, 121.819473, -1123.19995, 4, 2, 42, 420
            -68.5081787, 122.624321, -1123.19995, 2, 1, 3, 7
            160.233093, 58.9596481, -1123.19995, 4
            124.91214, 39.9765129, -1123.19995, 4
            )";
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            FAIL() << "Failed to open file " << path;
        }
        file << content;
    }

    std::optional<TestDataLoader::PointsWithRegionIds> pointsWithRegionIds = TestDataLoader::readPointsWithRegionIds(path, 42);
    ASSERT_TRUE(pointsWithRegionIds.has_value());
    ASSERT_EQ(pointsWithRegionIds->event->eventId_, 42);
    ASSERT_EQ(pointsWithRegionIds->event->hostNumPoints_, 7);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->size(), 7);

    // Check the first point
    ASSERT_EQ(pointsWithRegionIds->event->hostXs_[0], -59.243309f);
    ASSERT_EQ(pointsWithRegionIds->event->hostYs_[0], 115.000717f);
    ASSERT_EQ(pointsWithRegionIds->event->hostZs_[0], -1123.19995f);

    // Check last point
    ASSERT_EQ(pointsWithRegionIds->event->hostXs_[6], 124.91214f);
    ASSERT_EQ(pointsWithRegionIds->event->hostYs_[6], 39.9765129f);
    ASSERT_EQ(pointsWithRegionIds->event->hostZs_[6], -1123.19995f);

    // Check the region ids
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(0)[0], 2);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(0)[1], 0); // Rest of the array zeroed

    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(2)[0], 21);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(2)[1], 37);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(2)[2], 0); // Rest of the array zeroed

    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(3)[0], 4);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(3)[1], 2);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(3)[2], 42);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(3)[3], 420);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(3)[4], 0); // Rest of the array zeroed

    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(6)[0], 4);
    ASSERT_EQ(pointsWithRegionIds->regionIdsByPointIndex->at(6)[1], 0); // Rest of the array zeroed

    // Remove the file
    ASSERT_EQ(std::remove(path.c_str()), 0);
}