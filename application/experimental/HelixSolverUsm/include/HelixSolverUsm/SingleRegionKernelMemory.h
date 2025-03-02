#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/KernelMemory.h"

#include <sycl/sycl.hpp>


class SingleRegionKernel;

class SingleRegionKernelMemory : public KernelMemory
{
public:
    SingleRegionKernelMemory(sycl::queue& queue);
    ~SingleRegionKernelMemory() override = default;

    void operator=(const SingleRegionKernelMemory&) = delete;

    void allocateOnDevice() override;
    void deallocateOnDevice() override;

    static constexpr u_int8_t Phi0MaxDivisionLevel = 10;   // TODO: Tune
    static constexpr u_int8_t QOverPtMaxDivisionLevel = 10;   // TODO: Tune
    static constexpr u_int8_t MaxDivisionLevel = std::max(Phi0MaxDivisionLevel, QOverPtMaxDivisionLevel);
    static constexpr u_int8_t MaxAccumulatorRegionStackSize = MaxDivisionLevel * 4;
    static constexpr u_int8_t MaxPointListsNum = MaxDivisionLevel + 2;
    static constexpr u_int16_t MaxPointsInRegion = 10000;   // TODO: Tune
    static constexpr u_int32_t MaxPointListsPointsNum = MaxPointsInRegion * MaxPointListsNum;    // TODO: This is max possible number of points in all lists combined. Can be tuned

private:
    u_int32_t* indexes_;   // Just in case we need to keep info about which points form a helix
    float* xs_;
    float* ys_;
    float* zs_;
    EventUsm::LayerNumber* layers_;
    u_int32_t* pointLists_;
    float* rs_;
    float* phis_;

    friend class SingleRegionKernel;
    friend class SingleHelixDetectionTest;
};
