#include "SplitterUsm/SplitterSettings.h"

#include <cmath>
#include <utility>
#include <algorithm>


SplitterSettings::Wedge::Wedge(u_int16_t id, float zAngleMin, float zAngleMax, float xAngleMin, float xAngleMax, float interactionRegionWidth)
: id_(id)
, zAngleMin_(zAngleMin)
, zAngleMax_(zAngleMax)
, xAngleMin_(xAngleMin)
, xAngleMax_(xAngleMax)
, interactionRegionWidth_(interactionRegionWidth) {}

bool SplitterSettings::Wedge::operator==(const Wedge& other) const
{
    constexpr float epsilon = 1e-6;
    return std::fabs(zAngleMin_ - other.zAngleMin_) < epsilon &&
            std::fabs(zAngleMax_ - other.zAngleMax_) < epsilon &&
            std::fabs(xAngleMin_ - other.xAngleMin_) < epsilon &&
            std::fabs(xAngleMax_ - other.xAngleMax_) < epsilon &&
            std::fabs(interactionRegionWidth_ - other.interactionRegionWidth_) < epsilon;
}

bool SplitterSettings::Wedge::isValid() const
{
    constexpr float epsilon = 1e-6;
    return zAngleMin_ >= 0.0 && zAngleMin_ < 2.0 * M_PI + epsilon &&
            zAngleMax_ >= 0.0 && zAngleMax_ < 2.0 * M_PI + epsilon &&
            ((zAngleMax_ - zAngleMin_ < M_PI && zAngleMax_ > zAngleMin_) || (zAngleMax_ + 2.0 * M_PI - zAngleMin_ < M_PI && zAngleMax_ < zAngleMin_)) &&
            xAngleMin_ > 0.0 && xAngleMin_ < M_PI + epsilon &&
            xAngleMax_ > 0.0 && xAngleMax_ < M_PI + epsilon &&
            interactionRegionWidth_ > 0.0;
}


SplitterSettings::PoleRegion::PoleRegion(u_int16_t id, float xAngle, float interactionRegionWidth)
: id_(id)
, xAngle_(xAngle)
, interactionRegionWidth_(interactionRegionWidth) {}

bool SplitterSettings::PoleRegion::operator==(const PoleRegion& other) const
{
    constexpr float epsilon = 1e-6;
    return id_ == other.id_ &&
            std::fabs(xAngle_ - other.xAngle_) < epsilon &&
            std::fabs(interactionRegionWidth_ - other.interactionRegionWidth_) < epsilon;
}

bool SplitterSettings::PoleRegion::isValid() const
{
    constexpr float epsilon = 1e-6;
    return  id_ > 0 &&
            xAngle_ > 0.0 &&
            xAngle_ < M_PI + epsilon &&
            (xAngle_ < M_PI_2 || xAngle_ > M_PI_2) &&
            interactionRegionWidth_ > 0.0;
}


SplitterSettings::SplitterSettings(float maxAbsXy, float maxAbsZ, float minZAngle, float maxZAngle, float minXAgle, float maxXAgle, float poleRegionAngle, float interactionRegionMin, float interactionRegionMax, float zAngleMargin, float xAngleMargin, u_int8_t numZRanges, u_int8_t numXRanges, std::vector<Wedge>&& wedges, std::vector<PoleRegion>&& poleRegions)
: maxAbsXy_(maxAbsXy)
, maxAbsZ_(maxAbsZ)
, minZAngle_(minZAngle)
, maxZAngle_(maxZAngle)
, minXAgle_(minXAgle)
, maxXAgle_(maxXAgle)
, poleRegionAngle_(poleRegionAngle)
, interactionRegionMin_(interactionRegionMin)
, interactionRegionMax_(interactionRegionMax)
, zAngleMargin_(zAngleMargin)
, xAngleMargin_(xAngleMargin)
, numZRanges_(numZRanges)
, numXRanges_(numXRanges)
, wedges_(std::move(wedges))
, poleRegions_(std::move(poleRegions))
{
    if (wedges_.empty())
    {
        u_int16_t regionId = 0;
        std::vector<Range> zRanges = angleWrapPi(uniformRangeSplit(numZRanges, 0.0, 2.0 * M_PI, zAngleMargin));
        std::vector<Range> xRanges = uniformRangeSplit(numXRanges, minXAgle, M_PI - minXAgle, xAngleMargin);
        const float interactionRegionWidth = interactionRegionMax - interactionRegionMin;
        for(auto& xRange : xRanges)
        {
            for(auto& zRange : zRanges)
            {
                regionId++;
                wedges_.emplace_back(regionId, zRange.first, zRange.second, xRange.first, xRange.second, interactionRegionWidth);
            }
        }
    }

    if (poleRegions_.empty())
    {
        u_int16_t regionId = std::max_element(wedges_.begin(), wedges_.end(), [](const Wedge& a, const Wedge& b) { return a.id_ < b.id_; })->id_;

        regionId++;
        poleRegions_.emplace_back(regionId, M_PI - poleRegionAngle - xAngleMargin, interactionRegionMax - interactionRegionMin);
        regionId++;
        poleRegions_.emplace_back(regionId, poleRegionAngle + xAngleMargin, interactionRegionMax - interactionRegionMin);
    }
}

bool SplitterSettings::operator==(const SplitterSettings& other) const
{
    constexpr float epsilon = 1e-6;
    return std::fabs(maxAbsXy_ - other.maxAbsXy_) < epsilon &&
            std::fabs(maxAbsZ_ - other.maxAbsZ_) < epsilon &&
            std::fabs(minZAngle_ - other.minZAngle_) < epsilon &&
            std::fabs(maxZAngle_ - other.maxZAngle_) < epsilon &&
            std::fabs(minXAgle_ - other.minXAgle_) < epsilon &&
            std::fabs(maxXAgle_ - other.maxXAgle_) < epsilon &&
            std::fabs(poleRegionAngle_ - other.poleRegionAngle_) < epsilon &&
            std::fabs(interactionRegionMin_ - other.interactionRegionMin_) < epsilon &&
            std::fabs(interactionRegionMax_ - other.interactionRegionMax_) < epsilon &&
            std::fabs(zAngleMargin_ - other.zAngleMargin_) < epsilon &&
            std::fabs(xAngleMargin_ - other.xAngleMargin_) < epsilon &&
            numZRanges_ == other.numZRanges_ &&
            numXRanges_ == other.numXRanges_ &&
            wedges_ == other.wedges_ &&
            poleRegions_ == other.poleRegions_;
}

bool SplitterSettings::isValid() const
{
    constexpr float epsilon = 1e-6;
    return maxAbsXy_ > 0.0 && maxAbsZ_ > 0.0 &&
            minZAngle_ >= 0.0 && minZAngle_ < 2.0 * M_PI + epsilon &&
            maxZAngle_ >= 0.0 && maxZAngle_ < 2.0 * M_PI + epsilon &&
            minZAngle_ < maxZAngle_ &&
            minXAgle_ > 0.0 && minXAgle_ < M_PI &&
            maxXAgle_ > 0.0 && maxXAgle_ < M_PI &&
            minXAgle_ < maxXAgle_ &&
            zAngleMargin_ > 0.0 && xAngleMargin_ > 0.0 &&
            poleRegionAngle_ > 0.0 && poleRegionAngle_ < M_PI &&
            interactionRegionMin_ < 0.0 && interactionRegionMax_ > 0.0 &&
            numZRanges_ > 0 && numXRanges_ > 0 &&
            !wedges_.empty() &&
            std::all_of(wedges_.begin(), wedges_.end(), [](const Wedge& wedge) { return wedge.isValid(); }) &&
            !poleRegions_.empty() &&
            std::all_of(poleRegions_.begin(), poleRegions_.end(), [](const PoleRegion& poleRegion) { return poleRegion.isValid(); });
}

std::vector<SplitterSettings::Range> SplitterSettings::uniformRangeSplit(u_int8_t numRanges, float minValue, float maxValue, float margin)
{
    std::vector<float> boundaries;
    for (int i = 0; i < numRanges + 1; ++i)
    {
        boundaries.push_back(minValue + i * (maxValue - minValue) / numRanges);
    }

    std::vector<Range> ranges;
    for (int i = 0; i < numRanges; ++i)
    {
        ranges.push_back({boundaries[i] - margin, boundaries[i + 1] + margin});
    }

    return ranges;
}

float SplitterSettings::angleWrap2Pi(float angle)
{
    return std::fmod(angle, 2 * M_PI) + (angle >= 0 ? 0 : 2 * M_PI);
}

std::vector<SplitterSettings::Range> SplitterSettings::angleWrapPi(std::vector<Range>&& ranges)
{
    for (auto& range : ranges)
    {
        range.first = angleWrap2Pi(range.first);
        range.second = angleWrap2Pi(range.second);
    }
    return ranges;
}