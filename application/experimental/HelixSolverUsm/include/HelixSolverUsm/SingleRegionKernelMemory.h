#pragma once

#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "EventUsm/KernelMemory.h"

#include <sycl/sycl.hpp>


class SingleRegionKernel;

class SingleRegionKernelMemory : public KernelMemory
{
public:
    SingleRegionKernelMemory(sycl::queue& queue);
    ~SingleRegionKernelMemory() override = default;

    void operator=(const SingleRegionKernelMemory&) = delete;

    static constexpr u_int8_t Phi0MaxDivisionLevel = 8;   // TODO: Tune
    static constexpr u_int8_t QOverPtMaxDivisionLevel = 8;   // TODO: Tune
    static constexpr u_int8_t MaxDivisionLevel = std::max(Phi0MaxDivisionLevel, QOverPtMaxDivisionLevel);
    static constexpr u_int8_t MaxAccumulatorRegionStackSize = MaxDivisionLevel * 4;
    static constexpr u_int8_t MaxPointListsNum = MaxDivisionLevel + 2;
    static constexpr u_int16_t MaxPointsInRegion = 10000;   // TODO: Tune
    static constexpr u_int32_t MaxPointListsPointsNum = MaxPointsInRegion * MaxPointListsNum;    // TODO: This is max possible number of points in all lists combined. Can be tuned

    u_int32_t* indexes_ = nullptr;   // Just in case we need to keep info about which points form a helix
    float* xs_ = nullptr;
    float* ys_ = nullptr;
    float* zs_ = nullptr;
    EventUsm::LayerNumber* layers_ = nullptr;
    u_int32_t* pointLists_ = nullptr;
    float* rs_ = nullptr;
    float* phis_ = nullptr;

protected:
    void allocateInternal() override;
    void deallocateInternal() override;
};
