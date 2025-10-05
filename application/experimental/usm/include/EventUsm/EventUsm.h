#pragma once

#include "EventUsm/TransferableData.h"

#include <sycl/sycl.hpp>
#include <memory>


class EventUsm : public TransferableData
{
public:
    using EventId = u_int32_t;
    using LayerNumber = u_int8_t;

    class EventKernelMemory : public KernelMemory
    {
    public:
        EventKernelMemory(sycl::queue& queue);
        ~EventKernelMemory() override = default;

        EventKernelMemory(const EventKernelMemory&) = delete;

        u_int32_t* numPoints_ = nullptr;
        float* xs_ = nullptr;
        float* ys_ = nullptr;
        float* zs_ = nullptr;
        LayerNumber* layers_ = nullptr;

    protected:
        void allocateInternal() override;
        void deallocateInternal() override;
    };

    EventUsm(EventId eventId);
    EventUsm(const EventUsm&) = delete;
    ~EventUsm() override;

    void operator=(const EventUsm&) = delete;

    TransferableData::TransferEvents transferToDevice() override;
    TransferableData::TransferEvents transferToHost() override;

    static void copyHostData(const EventUsm& source, EventUsm& destination);

    static constexpr u_int32_t MaxPoints = 1e5;

    EventId eventId_;
    u_int32_t hostNumPoints_ = 0;
    float* hostXs_;
    float* hostYs_;
    float* hostZs_;
    LayerNumber* hostLayers_;
    EventKernelMemory* kernelMemory_;

protected:
    void setKernelMemoryInternal(KernelMemory* kernelMemory) override;
};
