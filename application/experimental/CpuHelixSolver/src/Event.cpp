#include "CpuHelixSolver/Event.h"

#include <algorithm>

Event::Event(uint32_t eventId)
: eventId_(eventId)
{
    xs_ = new float[MaxPoints];
    ys_ = new float[MaxPoints];
    zs_ = new float[MaxPoints];
}

Event::Event(Event&& other)
: eventId_(other.eventId_)
, xs_(std::exchange(other.xs_, nullptr))
, ys_(std::exchange(other.ys_, nullptr))
, zs_(std::exchange(other.zs_, nullptr)) {}

Event::~Event()
{
    if (xs_ == nullptr)
    {
        return;
    }

    delete[] xs_;
    delete[] ys_;
    delete[] zs_;
}

Event& Event::operator=(Event&& other)
{
    if (this == &other)
    {
        return *this;
    }

    eventId_ = other.eventId_;
    xs_ = std::exchange(other.xs_, nullptr);
    ys_ = std::exchange(other.ys_, nullptr);
    zs_ = std::exchange(other.zs_, nullptr);

    return *this;
}
