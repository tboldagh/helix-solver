#pragma once

#include <cstdint>

class Event
{
public:
    Event(uint32_t eventId = 0);
    Event(const Event& other) = delete;
    Event(Event&& other);
    ~Event();

    Event& operator=(const Event& other) = delete;
    Event& operator=(Event&& other);

    static constexpr uint32_t MaxPoints = 1e5;

    uint32_t eventId_;
    uint32_t numPoints_ = 0;
    float* xs_ = nullptr;
    float* ys_ = nullptr;
    float* zs_ = nullptr;
};