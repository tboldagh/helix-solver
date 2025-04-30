#pragma once

#include <cstdint>

class Event
{
public:
    Event();
    ~Event();

    static constexpr uint32_t MaxPoints = 1e5;

    uint32_t eventId_;
    uint32_t numPoints_ = 0;
    float* xs_ = nullptr;
    float* ys_ = nullptr;
    float* zs_ = nullptr;
};