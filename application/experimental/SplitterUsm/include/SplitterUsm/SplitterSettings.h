#pragma once

#include "ConstSizeVector/ConstSizeVector.h"

#include <CL/sycl.hpp>
#include <vector>


class SplitterSettings
{
public:
    class Wedge
    {
    public:
        Wedge() {}; // Clang bug: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-of-enclosing-class-outside
        Wedge(u_int16_t id, float zAngleMin, float zAngleMax, float xAngleMin, float xAngleMax, float interactionRegionWidth);
        Wedge(const Wedge&) = default;
        Wedge(Wedge&&) = default;
        Wedge& operator=(const Wedge&) = default;
        Wedge& operator=(Wedge&&) = default;

        bool operator==(const Wedge& other) const;
        bool operator!=(const Wedge& other) const { return !(*this == other); }
        bool isValid() const;   // See SplitterNotebook.ipynb notes

        u_int16_t id_ = 0;
        float zAngleMin_ = 0;
        float zAngleMax_ = 0;
        float xAngleMin_ = 0;
        float xAngleMax_ = 0;
        float interactionRegionWidth_ = 0;
    };

    class PoleRegion
    {
    public:
        PoleRegion() {}; // Clang bug: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-of-enclosing-class-outside
        PoleRegion(u_int16_t id, float xAngle, float interactionRegionWidth);
        PoleRegion(const PoleRegion&) = default;
        PoleRegion(PoleRegion&&) = default;
        PoleRegion& operator=(const PoleRegion&) = default;
        PoleRegion& operator=(PoleRegion&&) = default;

        bool operator==(const PoleRegion& other) const;
        bool operator!=(const PoleRegion& other) const { return !(*this == other); }
        bool isValid() const;   // See SplitterNotebook.ipynb notes

        u_int16_t id_ = 0;
        float xAngle_ = 0;
        float interactionRegionWidth_ = 0;
    };

    static constexpr u_int8_t MaxRegionsPerPoint = 32;
    static constexpr u_int16_t MaxWedgesNum = 1024;
    using Range = std::pair<float, float>;

    SplitterSettings() {}; // Clang bug: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-of-enclosing-class-outside
    SplitterSettings(float maxAbsXy, float maxAbsZ, float minZAngle, float maxZAngle, float minXAgle, float maxXAgle, float poleRegionAngle, float interactionRegionMin, float interactionRegionMax, float zAngleMargin, float xAngleMargin, u_int8_t numZRanges, u_int8_t numXRanges, ConstSizeVector<Wedge, MaxWedgesNum>&& wedges = {}, ConstSizeVector<PoleRegion, 2>&& poleRegions = {});
    SplitterSettings(const SplitterSettings&) = default;
    SplitterSettings(SplitterSettings&&) = default;
    SplitterSettings& operator=(const SplitterSettings&) = default;
    SplitterSettings& operator=(SplitterSettings&&) = default;
    
    bool operator==(const SplitterSettings& other) const;
    bool operator!=(const SplitterSettings& other) const { return !(*this == other); }
    bool isValid() const;   // See SplitterNotebook.ipynb notes

    // Detector properties
    float maxAbsXy_;
    float maxAbsZ_;

    // Splitter properties
    float minZAngle_;
    float maxZAngle_;
    float minXAgle_;
    float maxXAgle_;
    float poleRegionAngle_;
    float interactionRegionMin_;
    float interactionRegionMax_;
    float zAngleMargin_;
    float xAngleMargin_;
    u_int8_t numZRanges_;
    u_int8_t numXRanges_;

    // Regions
    ConstSizeVector<Wedge, MaxWedgesNum> wedges_;
    ConstSizeVector<PoleRegion, 2> poleRegions_;

private:
    static std::vector<Range> uniformRangeSplit(u_int8_t numRanges, float minValue, float maxValue, float margin);
    static float angleWrap2Pi(float angle);
    static std::vector<Range> angleWrapPi(std::vector<Range>&& ranges);
};