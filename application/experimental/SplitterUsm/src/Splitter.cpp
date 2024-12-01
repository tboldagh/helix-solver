#include "SplitterUsm/Splitter.h"

#include <cmath>
#include <CL/sycl.hpp>
#include <tuple>


Splitter::Splitter(const SplitterSettings& settings)
: settings_(settings) {}

void Splitter::getRegionIds(float x, float y, float z, RegionIds& regionIds) const
{
    getRegionIdsNaive(x, y, z, regionIds);
}

u_int16_t Splitter::getNumRegions() const
{
    return settings_.numZRanges_ * settings_.numXRanges_ + 2;   // + 2 for poles
}

const SplitterSettings& Splitter::getSettings() const
{
    return settings_;
}

bool Splitter::isPointInRegion(float x, float y, float z, u_int16_t regionId) const
{
    const auto lastWedgeId = settings_.numZRanges_ * settings_.numXRanges_; // 0 reserved for invalid region
    if (regionId <= lastWedgeId)
    {
        const auto& wedge = settings_.wedges_[regionId - 1];
        return isPointInWedge(x, y, z, wedge);
    }
    else
    {
        const auto& poleRegion = settings_.poleRegions_[regionId - lastWedgeId - 1];
        return isPointInPoleRegion(x, y, z, poleRegion);
    }
}

void Splitter::getRegionIdsNaive(float x, float y, float z, RegionIds& regionIds) const
{
    // TODO: Assert regionIds is zeroed

    u_int8_t inRegionCount = 0;

    for (auto i = 0; i < settings_.poleRegions_.getSize(); i++)
    {
        const auto& poleRegion = settings_.poleRegions_[i];
        if (isPointInPoleRegion(x, y, z, poleRegion))
        {
            regionIds[inRegionCount++] = poleRegion.id_;
        }
    }

    for (auto i = 0; i < settings_.wedges_.getSize(); i++)
    {
        const auto& wedge = settings_.wedges_[i];
        if (isPointInWedge(x, y, z, wedge))
        {
            regionIds[inRegionCount++] = wedge.id_;
        }
    }
}

bool Splitter::isPointInPoleRegion(float x, float y, float z, const SplitterSettings::PoleRegion& poleRegion) const
{
    float xPlaneOutermostY, xPlaneOutermostZ;
    float xPlaneDirectionZ = sycl::cos(poleRegion.xAngle_);
    float xPlaneDirectionY = sycl::sin(poleRegion.xAngle_);

    float xPlaneScaleToZLimit = sycl::abs(xPlaneDirectionZ) / settings_.maxAbsZ_;
    float xPlaneScaleToYLimit = sycl::abs(xPlaneDirectionY) / settings_.maxAbsXy_;
    float scale = 1 / sycl::max(xPlaneScaleToZLimit, xPlaneScaleToYLimit);

    xPlaneOutermostY = xPlaneDirectionY * scale;
    xPlaneOutermostZ = xPlaneDirectionZ * scale;

    float interactionRegionShift = (-1.f) * (poleRegion.xAngle_ < M_PI / 2.f ? 1.f : -1.f) * poleRegion.interactionRegionWidth_ / 2.f;
    float boundaryToZAngle = sycl::atan2(xPlaneOutermostY, xPlaneOutermostZ - interactionRegionShift);
    float distanceToZ = sycl::sqrt(x * x + y * y);
    float zShifted = z - interactionRegionShift;
    float pointToZAngle = sycl::atan2(distanceToZ, zShifted);

    if ((poleRegion.xAngle_ < M_PI / 2.f && pointToZAngle > boundaryToZAngle) || (poleRegion.xAngle_ > M_PI / 2.f && pointToZAngle < boundaryToZAngle))
    {
        return false;
    }

    return true;
}

bool Splitter::isPointInWedge(float x, float y, float z, const SplitterSettings::Wedge& wedge) const
{
    return isPointInWedgeZAngle(x, y, z, wedge) && isPointInWedgeXAngle(x, y, z, wedge);
}

bool Splitter::isPointInWedgeZAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const
{
    const float zAngle = atan2Wrap2Pi(y, x);
    if (wedge.zAngleMin_ < wedge.zAngleMax_) {
        if (zAngle < wedge.zAngleMin_ || zAngle > wedge.zAngleMax_) {
            return false;
        }
    } else {
        // Wedge crosses zAngle = 0
        if ((zAngle <= M_PI && zAngle > wedge.zAngleMax_) || (zAngle > M_PI && zAngle < wedge.zAngleMin_)) {
            return false;
        }
    }
    return true;
}

bool Splitter::isPointInWedgeXAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const
{

    const auto outermostPoint = [this, &wedge](float xAngle) -> std::tuple<float, float, float> {
        const float xPlaneDirectionZ = sycl::cos(xAngle);
        const float xPlaneDirectionY = sycl::sin(xAngle);

        const float xPlaneScaleToZLimit = sycl::abs(xPlaneDirectionZ) / settings_.maxAbsZ_;
        const float xPlaneScaleToYLimit = sycl::abs(xPlaneDirectionY) / settings_.maxAbsXy_;
        const float scale = 1.0f / sycl::max(xPlaneScaleToZLimit, xPlaneScaleToYLimit);

        const float x = xPlaneDirectionY * sycl::cos(wedge.zAngleMin_) * scale;
        const float y = xPlaneDirectionY * sycl::sin(wedge.zAngleMin_) * scale;
        const float z = xPlaneDirectionZ * scale;

        return { x, y, z };
    };

    const auto toZAngle = [](float x, float y, float z, float interactionRegionShift) -> float {
        const float distanceToZ = sycl::sqrt(x * x + y * y);
        return sycl::atan2(distanceToZ, z - interactionRegionShift);
    };

    // Check boundary condition for xAngleMin
    float interactionRegionShift = wedge.interactionRegionWidth_ / 2.0f;
    auto [outermostX, outermostY, outermostZ] = outermostPoint(wedge.xAngleMin_);
    float boundaryToZAngle = toZAngle(outermostX, outermostY, outermostZ, interactionRegionShift);
    float pointToZAngle = toZAngle(x, y, z, interactionRegionShift);
    if (pointToZAngle < boundaryToZAngle) {
        return false;
    }

    // Check boundary condition for xAngleMax
    interactionRegionShift = -wedge.interactionRegionWidth_ / 2.0f;
    std::tie(outermostX, outermostY, outermostZ) = outermostPoint(wedge.xAngleMax_);
    boundaryToZAngle = toZAngle(outermostX, outermostY, outermostZ, interactionRegionShift);
    pointToZAngle = toZAngle(x, y, z, interactionRegionShift);
    if (pointToZAngle > boundaryToZAngle) {
        return false;
    }

    return true;
}

float Splitter::atan2Wrap2Pi(float y, float x)
{
    return wrap2Pi(sycl::atan2(y, x));
}

float Splitter::wrap2Pi(float angle)
{
    return angle - 2.0 * M_PI * sycl::floor(angle / (2.0 * M_PI));
}
