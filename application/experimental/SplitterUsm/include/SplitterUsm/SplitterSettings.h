#pragma once

#include <CL/sycl.hpp>
#include <vector>

class SplitterSettings
{
public:
    class Wedge
    {
    public:
        Wedge(u_int16_t id, float zAngleMin, float zAngleMax, float xAngleMin, float xAngleMax, float interactionRegionWidth);
        bool operator==(const Wedge& other) const;
        bool isValid() const;   // See SplitterNotebook.ipynb notes

        const u_int16_t id_ = 0;
        const float zAngleMin_ = 0;
        const float zAngleMax_ = 0;
        const float xAngleMin_ = 0;
        const float xAngleMax_ = 0;
        const float interactionRegionWidth_ = 0;
    };

    class PoleRegion
    {
    public:
        PoleRegion(u_int16_t id, float xAngle, float interactionRegionWidth);
        bool operator==(const PoleRegion& other) const;
        bool isValid() const;   // See SplitterNotebook.ipynb notes

        const u_int16_t id_ = 0;
        const float xAngle_ = 0;
        const float interactionRegionWidth_ = 0;
    };

    using Range = std::pair<float, float>;

    SplitterSettings(float maxAbsXy, float maxAbsZ, float minZAngle, float maxZAngle, float minXAgle, float maxXAgle, float poleRegionAngle, float interactionRegionMin, float interactionRegionMax, float zAngleMargin, float xAngleMargin, u_int8_t numZRanges, u_int8_t numXRanges, std::vector<Wedge>&& wedges = {}, std::vector<PoleRegion>&& poleRegions = {});
    bool operator==(const SplitterSettings& other) const;
    bool isValid() const;   // See SplitterNotebook.ipynb notes

    // Detector properties
    const float maxAbsXy_;
    const float maxAbsZ_;

    // Splitter properties
    const float minZAngle_;
    const float maxZAngle_;
    const float minXAgle_;
    const float maxXAgle_;
    const float poleRegionAngle_;
    const float interactionRegionMin_;
    const float interactionRegionMax_;
    const float zAngleMargin_;
    const float xAngleMargin_;
    const u_int8_t numZRanges_;
    const u_int8_t numXRanges_;

    // Regions
    std::vector<Wedge> wedges_;
    std::vector<PoleRegion> poleRegions_;

private:
    static std::vector<Range> uniformRangeSplit(u_int8_t numRanges, float minValue, float maxValue, float margin);
    static float angleWrap2Pi(float angle);
    static std::vector<Range> angleWrapPi(std::vector<Range>&& ranges);
};