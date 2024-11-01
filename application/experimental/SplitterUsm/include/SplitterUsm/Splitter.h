#pragma once

#include "SplitterUsm/SplitterSettings.h"

#include <CL/sycl.hpp>


class Splitter
{
public:
    static constexpr u_int8_t MaxRegionsPerPoint = 32;
    using RegionIds = std::array<u_int16_t, MaxRegionsPerPoint>;

    Splitter(const SplitterSettings& settings);

    void getRegionIds(float x, float y, float z, RegionIds& regionIds) const;

private:
    void getRegionIdsNaive(float x, float y, float z, RegionIds& regionIds) const;
    bool isPointInPoleRegion(float x, float y, float z, const SplitterSettings::PoleRegion& poleRegion) const;
    bool isPointInWedge(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;
    bool isPointInWedgeZAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;
    bool isPointInWedgeXAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;

    static float atan2Wrap2Pi(float y, float x);
    static float wrap2Pi(float angle);

    const SplitterSettings settings_;
};