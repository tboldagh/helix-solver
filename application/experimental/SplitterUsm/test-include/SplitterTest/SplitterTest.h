#pragma once

#include "SplitterUsm/Splitter.h"
#include "SplitterUsm/SplitterSettings.h"
#include "EventUsm/EventUsm.h"
#include "SplitterUsm/TestDataLoader.h"

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cmath>

class SplitterTest : public ::testing::Test
{
protected:
    SplitterTest();
    ~SplitterTest() override = default;

    SplitterSettings getSplitterSettings();
    void writeSplitterSettings();
    static void writePoints(const std::string& path, const std::string& content);
    static void runTestDataGenerator(const std::string& pointsPath, const std::string& pointsWithRegionIdPath);
    void readPointsWithRegionIds(const std::string& path, EventUsm::EventId eventId);
    void prepareTestData(const std::string& pointsPath, const std::string& pointsWithRegionIdPath);
    bool regionIdsEqual(const Splitter::RegionIds& actual, const Splitter::RegionIds& expected);

    const SplitterSettings splitterSettings_;
    const Splitter splitter_;
    static const std::string SandboxPath;
    static const std::string SplitterSettingsPath;
    static const std::string TestDataGeneratorPath;
    TestDataLoader::PointsWithRegionIds pointsWithRegionIds_;
};