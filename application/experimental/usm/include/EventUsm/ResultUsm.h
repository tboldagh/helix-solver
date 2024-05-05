#pragma once

#include "EventUsm/DataUsm.h"


#include <CL/sycl.hpp>
#include <memory>


class ResultUsm : public DataUsm
{
public:
    using ResultId = u_int32_t;

    static constexpr u_int32_t MaxSolutions = 1e5;

    ResultUsm(ResultId resultId);
    ~ResultUsm();

    bool allocateOnDevice(sycl::queue& queue) override;
    bool deallocateOnDevice(sycl::queue& queue) override;
    DataUsm::TransferEvents transferToDevice(sycl::queue& queue) override;
    DataUsm::TransferEvents transferToHost(sycl::queue& queue) override;

    bool takeResourceGroup(const DeviceResourceGroup& resourceGroup, const sycl::queue& queue) override;
    std::pair<std::unique_ptr<DeviceResourceGroup>, const sycl::queue*> releaseResourceGroup() override;

    // Allocates resources on device without host data.
    static std::unique_ptr<DeviceResourceGroup> allocateDeviceResources(sycl::queue& queue);
    // Deallocates resources on device without host data.
    static void deallocateDeviceResources(DeviceResourceGroup& resourceGroup, sycl::queue& queue);

    // Direct access to frequently accessed data for performance reasons.
    // Manipulate data under the pointers but don't manage memory directly.
    ResultId resultId_;

    u_int32_t hostNumSolutions_ = 0;
    float hostSomeSolutionParameters_[MaxSolutions];

    u_int32_t* deviceNumSolutions_ = nullptr;
    float* deviceSomeSolutionParameters_ = nullptr;
};