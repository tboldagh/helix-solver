#pragma once

#include "EventUsm/DataUsm.h"

#include <CL/sycl.hpp>
#include <memory>


class EventUsm : public DataUsm
{
public:
    using EventId = u_int32_t;
    using LayerNumber = u_int8_t;

    static constexpr u_int32_t MaxPoints = 1e5;

    EventUsm(EventId eventId);
    EventUsm(const EventUsm&) = delete;
    ~EventUsm() override;

    void operator=(const EventUsm&) = delete;

    bool allocateOnDevice(sycl::queue& queue) override;
    bool deallocateOnDevice(sycl::queue& queue) override;
    DataUsm::TransferEvents transferToDevice(sycl::queue& queue) override;
    DataUsm::TransferEvents transferToHost(sycl::queue& queue) override;

    bool takeResourceGroup(const DeviceResourceGroup& resourceGroup, const sycl::queue& queue) override;
    std::pair<std::unique_ptr<DeviceResourceGroup>, const sycl::queue*> releaseResourceGroup() override;

    // Allocates resources on device without host data.
    static std::unique_ptr<DeviceResourceGroup> allocateDeviceResources(sycl::queue& queue);
    // Deallocates resources on device without host data.
    static void deallocateDeviceResources(const DeviceResourceGroup& resourceGroup, sycl::queue& queue);

    static void copyHostData(const EventUsm& source, EventUsm& destination);

    // Direct access to frequently accessed data for performance reasons.
    // Manipulate data under the pointers but don't manage memory directly.
    EventId eventId_;

    u_int32_t hostNumPoints_ = 0;
    float hostXs_[MaxPoints];
    float hostYs_[MaxPoints];
    float hostZs_[MaxPoints];
    LayerNumber hostLayers_[MaxPoints];

    u_int32_t* deviceNumPoints_ = nullptr;
    float* deviceXs_ = nullptr;
    float* deviceYs_ = nullptr;
    float* deviceZs_ = nullptr;
    LayerNumber* deviceLayers_ = nullptr;
};
