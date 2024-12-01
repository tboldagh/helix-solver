#pragma once

#include "SplitterUsm/SplitterSettings.h"

#include <CL/sycl.hpp>


class Splitter
{
public:
    using RegionIds = std::array<u_int16_t, SplitterSettings::MaxRegionsPerPoint>;

    Splitter() {}; // Clang bug: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-of-enclosing-class-outside
    Splitter(const SplitterSettings& settings);
    Splitter(const Splitter&) = default;
    Splitter(Splitter&&) = default;
    Splitter& operator=(const Splitter&) = default;
    Splitter& operator=(Splitter&&) = default;

    SYCL_EXTERNAL void getRegionIds(float x, float y, float z, RegionIds& regionIds) const;
    SYCL_EXTERNAL bool isPointInRegion(float x, float y, float z, u_int16_t regionId) const;
    SYCL_EXTERNAL u_int16_t getNumRegions() const;
    SYCL_EXTERNAL const SplitterSettings& getSettings() const;

private:
    void getRegionIdsNaive(float x, float y, float z, RegionIds& regionIds) const;
    bool isPointInPoleRegion(float x, float y, float z, const SplitterSettings::PoleRegion& poleRegion) const;
    bool isPointInWedge(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;
    bool isPointInWedgeZAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;
    bool isPointInWedgeXAngle(float x, float y, float z, const SplitterSettings::Wedge& wedge) const;

    static float atan2Wrap2Pi(float y, float x);
    static float wrap2Pi(float angle);

    SplitterSettings settings_;
};