#pragma once

#include <cstdint>


namespace DataTypes
{
class Spacepoint
{
public:
    Spacepoint(uint32_t eventId, uint32_t measurementId, uint32_t geometryId, float x, float y, float z, float varR, float varZ);
    Spacepoint(float x, float y, float z);

    uint32_t eventId_;
    uint32_t measurementId_;
    uint32_t geometryId_;
    float x_;
    float y_;
    float z_;
    float varR_;
    float varZ_;
};
}   // namespace DataTypes