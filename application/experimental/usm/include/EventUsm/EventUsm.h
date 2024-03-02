#pragma once

#include <CL/sycl.hpp>

class EventUsm
{
public:
    using EventId = u_int32_t;
    using LayerNumber = u_int8_t;

    static constexpr u_int32_t MaxPoints = 1e5;

    EventUsm(EventId eventId);
    ~EventUsm();

    bool allocateOnDevice(sycl::queue& queue);
    bool transferToDevice(sycl::queue& queue);
    bool transferToHost(sycl::queue& queue);

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

private:
    bool deallocateOnDevice(sycl::queue& queue);

    bool allocated_ = false;
};
