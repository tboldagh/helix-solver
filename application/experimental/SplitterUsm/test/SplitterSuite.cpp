#include "SplitterUsm/SplitterSettings.h"
#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "SplitterTest/SplitterTest.h"

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include <CL/sycl.hpp>
#include <cmath>
#include <unordered_set>


class SyclSplitterTest : public SplitterTest
{
protected:
    SyclSplitterTest()
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        queue_ = sycl::queue(sycl::gpu_selector_v);
    }

    ~SyclSplitterTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::ILoggerMock logger_;
    sycl::queue queue_;
};

TEST_F(SyclSplitterTest, Random10Points)
{
    EventUsm::EventId eventId = 42;
    auto event = std::make_unique<EventUsm>(eventId);
    std::array<float, 10> xs{1000, 200, -300, 420, -800, 0, 500, 600, -700, 800};
    std::array<float, 10> ys{300, 200, -300, 300, 90, 20, 0, 832, -500, 900};
    std::array<float, 10> zs{100, 0, 420, 2137, 1500, -2500, -400, 1800, -1900, 1000};
    std::array<EventUsm::LayerNumber, 10> layers{0, 1, 2, 3, 4, 0, 1, 2, 3, 4};
    for (u_int32_t i = 0; i < 10; ++i)
    {
        event->hostXs_[i] = xs[i];
        event->hostYs_[i] = ys[i];
        event->hostZs_[i] = zs[i];
        event->hostLayers_[i] = layers[i];
    }
    event->hostNumPoints_ = 10;

    event->allocateOnDevice(queue_);
    event->transferToDevice(queue_);

    auto deviceSplitter = sycl::malloc_device<Splitter>(1, queue_);
    queue_.memcpy(deviceSplitter, &splitter_, sizeof(Splitter)).wait();

    std::array<Splitter::RegionIds, 10> regionIds;
    auto deviceRegionIds = sycl::malloc_device<Splitter::RegionIds>(10, queue_);

    queue_.submit([&](sycl::handler& handler)
    {
        auto xs = event->deviceXs_;
        auto ys = event->deviceYs_;
        auto zs = event->deviceZs_;

        handler.parallel_for(sycl::range<1>(event->hostNumPoints_), [xs, ys, zs, deviceRegionIds, deviceSplitter](sycl::id<1> idx)
        {
            deviceSplitter->getRegionIds(xs[idx], ys[idx], zs[idx], deviceRegionIds[idx]);
        });
    }).wait();

    event->deallocateOnDevice(queue_);

    queue_.memcpy(regionIds.data(), deviceRegionIds, 10 * sizeof(Splitter::RegionIds)).wait();

    for (u_int32_t i = 0; i < 10; ++i)
    {
        Splitter::RegionIds expectedRegionIds{0};
        splitter_.getRegionIds(xs[i], ys[i], zs[i], expectedRegionIds);
        ASSERT_TRUE(regionIdsEqual(regionIds[i], expectedRegionIds));
    }

    sycl::free(deviceRegionIds, queue_);
}


class SinglePointTest : public SplitterTest
{
protected:
    SinglePointTest() = default;
    ~SinglePointTest() override = default;

    void writeSinglePointCsvAndPrepareTestData(float x, float y, float z)
    {
        std::string pointsCsv = "measurement_id, geometry_id, x, y, z\n"
                + std::to_string(0)
                + ", " + std::to_string(0)
                + ", " + std::to_string(x)
                + ", " + std::to_string(y)
                + ", " + std::to_string(z) + "\n";
        writePoints(PointsPath, pointsCsv);
        prepareTestData(PointsPath, PointsWithRegionIdPath);
    } 

    static const std::string PointsPath;
    static const std::string PointsWithRegionIdPath;
};
const std::string SinglePointTest::PointsPath = SandboxPath + "/SinglePointTest_points.csv";
const std::string SinglePointTest::PointsWithRegionIdPath = SandboxPath + "/SinglePointTest_points_with_region_ids.csv";


TEST_F(SinglePointTest, WedgeZAngle0ZPositive)
{
    const float zAngle = 0.0;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAngle0PlusZPositive)
{
    const float zAngle = 0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAngle0MinusZPositive)
{
    const float zAngle = -0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiZPositive)
{
    const float zAngle = M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiPlusZPositive)
{
    const float zAngle = 1.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiMinusZPositive)
{
    const float zAngle = 0.999 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAngle0ZNegative)
{
    const float zAngle = 0.0;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAngle0PlusZNegative)
{
    const float zAngle = 0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAngle0MinusZNegative)
{
    const float zAngle = -0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiZNegative)
{
    const float zAngle = M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiPlusZNegative)
{
    const float zAngle = 1.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, WedgeZAnglePiMinusZNegative)
{
    const float zAngle = 0.999 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle);
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle);
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAngle0PlusZPositive)
{
    const float zAngle = 0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAngle0MinusZPositive)
{
    const float zAngle = -0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAnglePiPlusZPositive)
{
    const float zAngle = 1.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAnglePiMinusZPositive)
{
    const float zAngle = 0.999 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = 2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAngle0PlusZNegative)
{
    const float zAngle = 0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAngle0MinusZNegative)
{
    const float zAngle = -0.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAnglePiPlusZNegative)
{
    const float zAngle = 1.001 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

TEST_F(SinglePointTest, PoleRegionZAnglePiMinusZNegative)
{
    const float zAngle = 0.999 * M_PI;
    const float x = splitterSettings_.maxAbsXy_* 0.9 * sycl::cos(zAngle) * 0.001;
    const float y = splitterSettings_.maxAbsXy_* 0.9 * sycl::sin(zAngle) * 0.001;
    const float z = -2000.0;
    writeSinglePointCsvAndPrepareTestData(x, y, z);

    Splitter::RegionIds regionIds{0};
    splitter_.getRegionIds(x, y, z, regionIds);
    regionIdsEqual(regionIds, pointsWithRegionIds_.regionIdsByPointIndex_->at(0));
}

class FullEventTest : public SplitterTest
{
protected:
    FullEventTest() = default;
    ~FullEventTest() override = default;

    static const std::string TestDataDir;
};
const std::string FullEventTest::TestDataDir = "/helix/repo/application/experimental/SplitterUsm/test-data";

TEST_F(FullEventTest, FullEvent0)
{
    const std::string pointsPath = TestDataDir + "/event_0.csv";
    const std::string pointsWithRegionIdPath = SandboxPath + "/FullEventTest_event_0_points_with_region_ids.csv";
    prepareTestData(pointsPath, pointsWithRegionIdPath);

    // Check if points CSV has some points
    ASSERT_GE(pointsWithRegionIds_.regionIdsByPointIndex_->size(), 100);

    for (const auto& [pointIndex, expectedRegionIds] : *pointsWithRegionIds_.regionIdsByPointIndex_)
    {
        Splitter::RegionIds regionIds{0};
        splitter_.getRegionIds(
            pointsWithRegionIds_.event_->hostXs_[pointIndex],
            pointsWithRegionIds_.event_->hostYs_[pointIndex],
            pointsWithRegionIds_.event_->hostZs_[pointIndex],
            regionIds
        );
        if (!regionIdsEqual(regionIds, expectedRegionIds))
        {
            std::cout << "Point [" << pointIndex << "]:\t"
                    << pointsWithRegionIds_.event_->hostXs_[pointIndex]
                    << ", " << pointsWithRegionIds_.event_->hostYs_[pointIndex]
                    << ", " << pointsWithRegionIds_.event_->hostZs_[pointIndex] << std::endl;
            std::cout << "\tExpected region ids: ";
            for (const auto& regionId : expectedRegionIds)
            {
                std::cout << regionId << " ";
            }
            std::cout << std::endl;
            std::cout << "\tActual region ids: ";
            for (const auto& regionId : regionIds)
            {
                std::cout << regionId << " ";
            }
            std::cout << std::endl;
            
            // std::terminate();
            FAIL();
        }
    }
}