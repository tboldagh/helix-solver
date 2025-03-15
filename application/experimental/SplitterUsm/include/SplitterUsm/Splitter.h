#pragma once

#include "SplitterUsm/SplitterSettings.h"
#include "EventUsm/TransferableData.h"

#include <sycl/sycl.hpp>


class Splitter : public TransferableData
{
public:
    using RegionIds = std::array<u_int16_t, SplitterSettings::MaxRegionsPerPoint>;

    class SplitterSettingsKernelMemory : public KernelMemory
    {
    public:
        SplitterSettingsKernelMemory(sycl::queue& queue);
        ~SplitterSettingsKernelMemory() override = default;

        SplitterSettingsKernelMemory(const SplitterSettingsKernelMemory&) = delete;

        SplitterSettings* settings_ = nullptr;

    protected:
        void allocateInternal() override;
        void deallocateInternal() override;
    };

    Splitter() {}; // Clang bug: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-of-enclosing-class-outside
    SYCL_EXTERNAL Splitter(const SplitterSettings& settings);
    Splitter(const Splitter& other);
    Splitter(Splitter&& other);
    ~Splitter() override = default;
    Splitter& operator=(const Splitter& other);
    Splitter& operator=(Splitter&& other);

    TransferableData::TransferEvents transferToDevice() override;
    TransferableData::TransferEvents transferToHost() override;

    SYCL_EXTERNAL void getRegionIds(float x, float y, float z, RegionIds& regionIds) const;
    SYCL_EXTERNAL bool isPointInRegion(float x, float y, float z, u_int16_t regionId) const;
    SYCL_EXTERNAL u_int16_t getNumRegions() const;
    SYCL_EXTERNAL const SplitterSettings& getSettings() const;

    SplitterSettingsKernelMemory* splitterSettingsKernelMemory_ = nullptr;

protected:
    void setKernelMemoryInternal(KernelMemory* kernelMemory) override;

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