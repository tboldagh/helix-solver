#include "SplitterTest/SplitterTest.h"

#include <fstream>
#include <string>


const std::string SplitterTest::SandboxPath = "/tmp/ut_sandbox";
const std::string SplitterTest::SplitterSettingsPath = SandboxPath + "/SplitterTest_splitterSettings.json";
const std::string SplitterTest::TestDataGeneratorPath = "/helix/repo/application/experimental/SplitterUsm/TestDataGenerator/TestDataGenerator.py";


SplitterTest::SplitterTest()
: splitterSettings_(getSplitterSettings())
, splitter_(splitterSettings_)
{
    writeSplitterSettings();
}

SplitterSettings SplitterTest::getSplitterSettings()
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

void SplitterTest::writeSplitterSettings()
{
    ASSERT_TRUE(TestDataLoader::writeSplitterSettings(SplitterSettingsPath, splitterSettings_));
}

void SplitterTest::writePoints(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    ASSERT_TRUE(file.is_open());
    file << content;
}

void SplitterTest::runTestDataGenerator(const std::string& pointsPath, const std::string& pointsWithRegionIdPath)
{
    const std::string command = "python3 " + TestDataGeneratorPath
            + " --splitter_settings " + SplitterSettingsPath
            + " --points " + pointsPath
            + " --points_output " + pointsWithRegionIdPath;
    ASSERT_EQ(std::system(command.c_str()), 0);
}

void SplitterTest::readPointsWithRegionIds(const std::string& path, EventUsm::EventId eventId)
{
    std::optional<TestDataLoader::PointsWithRegionIds> pointsWithRegionIdsOptional = TestDataLoader::readPointsWithRegionIds(path, eventId);
    ASSERT_TRUE(pointsWithRegionIdsOptional.has_value());
    pointsWithRegionIds_ = std::move(pointsWithRegionIdsOptional.value());
}

void SplitterTest::prepareTestData(const std::string& pointsPath, const std::string& pointsWithRegionIdPath)
{
    runTestDataGenerator(pointsPath, pointsWithRegionIdPath);
    constexpr EventUsm::EventId eventId = 0;
    readPointsWithRegionIds(pointsWithRegionIdPath, eventId);
}

bool SplitterTest::regionIdsEqual(const Splitter::RegionIds& actual, const Splitter::RegionIds& expected)
{
    std::unordered_set<u_int16_t> actualSet;
    for (const auto& regionId : actual)
    {
        if (regionId == 0)
        {
            break;
        }

        actualSet.insert(regionId);
    }

    std::unordered_set<u_int16_t> expectedSet;
    for (const auto& regionId : expected)
    {
        if (regionId == 0)
        {
            break;
        }

        expectedSet.insert(regionId);
    }

    return actualSet == expectedSet;
}