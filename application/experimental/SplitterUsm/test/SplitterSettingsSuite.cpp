#include "SplitterUsm/SplitterSettings.h"

#include <gtest/gtest.h>
#include <string>
#include <CL/sycl.hpp>
#include <cmath>


class SyclCompatibilityTest : public ::testing::Test
{
protected:
    SyclCompatibilityTest()
    {
        queue_ = sycl::queue(sycl::gpu_selector_v);
    }

    ~SyclCompatibilityTest() override = default;

    sycl::queue queue_;
};

TEST_F(SyclCompatibilityTest, WedgeTransferableToKernel)
{
    constexpr u_int16_t id = 42;
    constexpr float zAngleMin = 1.5 * M_PI;
    constexpr float zAngleMax = 2.f * M_PI;
    constexpr float xAngleMin = 0.5 * M_PI;
    constexpr float xAngleMax = M_PI;
    constexpr float interactionRegionWidth = 10.0;
    const SplitterSettings::Wedge wedge(id, zAngleMin, zAngleMax, xAngleMin, xAngleMax, interactionRegionWidth);
    ASSERT_TRUE(wedge.isValid());

    auto deviceWedge = sycl::malloc_device<SplitterSettings::Wedge>(1, queue_);
    queue_.memcpy(deviceWedge, &wedge, sizeof(SplitterSettings::Wedge)).wait();

    auto deviceWedgeCopy = sycl::malloc_device<SplitterSettings::Wedge>(1, queue_);

    queue_.submit([&](sycl::handler& handler)
    {
        handler.single_task<class WedgeTransferKernel>([=]()
        {
            deviceWedgeCopy[0] = deviceWedge[0];
        });
    });

    SplitterSettings::Wedge wedgeCopy;
    queue_.memcpy(&wedgeCopy, deviceWedgeCopy, sizeof(SplitterSettings::Wedge)).wait();
    ASSERT_EQ(wedge, wedgeCopy);
    ASSERT_TRUE(wedgeCopy.isValid());
}

TEST_F(SyclCompatibilityTest, PoleRegionTransferableToKernel)
{
    constexpr u_int16_t id = 42;
    constexpr float xAngle = 0.5 * M_PI;
    constexpr float interactionRegionWidth = 10.0;
    const SplitterSettings::PoleRegion poleRegion(id, xAngle, interactionRegionWidth);
    ASSERT_TRUE(poleRegion.isValid());

    auto devicePoleRegion = sycl::malloc_device<SplitterSettings::PoleRegion>(1, queue_);
    queue_.memcpy(devicePoleRegion, &poleRegion, sizeof(SplitterSettings::PoleRegion)).wait();

    auto devicePoleRegionCopy = sycl::malloc_device<SplitterSettings::PoleRegion>(1, queue_);

    queue_.submit([&](sycl::handler& handler)
    {
        handler.single_task<class PoleRegionTransferKernel>([=]()
        {
            devicePoleRegionCopy[0] = devicePoleRegion[0];
        });
    });

    SplitterSettings::PoleRegion poleRegionCopy;
    queue_.memcpy(&poleRegionCopy, devicePoleRegionCopy, sizeof(SplitterSettings::PoleRegion)).wait();
    ASSERT_EQ(poleRegion, poleRegionCopy);
    ASSERT_TRUE(poleRegionCopy.isValid());
}

TEST_F(SyclCompatibilityTest, SplitterSettingsTransferableToKernel)
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
    const SplitterSettings settings(maxAbsXy, maxAbsZ, minZAngle, maxZAngle, minXAgle, maxXAgle, poleRegionAngle, interactionRegionMin, interactionRegionMax, zAngleMargin, xAngleMargin, numZRanges, numXRanges);
    ASSERT_TRUE(settings.isValid());

    auto deviceSettings = sycl::malloc_device<SplitterSettings>(1, queue_);
    queue_.memcpy(deviceSettings, &settings, sizeof(SplitterSettings)).wait();

    auto deviceSettingsCopy = sycl::malloc_device<SplitterSettings>(1, queue_);

    queue_.submit([&](sycl::handler& handler)
    {
        handler.single_task<class SettingsTransferKernel>([=]()
        {
            deviceSettingsCopy[0] = deviceSettings[0];
        });
    }).wait();

    SplitterSettings settingsCopy;
    queue_.memcpy(&settingsCopy, deviceSettingsCopy, sizeof(SplitterSettings)).wait();

    ASSERT_EQ(settings, settingsCopy);
    ASSERT_TRUE(settingsCopy.isValid());
}