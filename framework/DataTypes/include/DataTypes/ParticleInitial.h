#pragma once

#include <cstdint>


namespace DataTypes
{
class ParticleInitial
{
public:
    ParticleInitial(uint32_t eventId, uint64_t particleId, uint32_t particleType, uint32_t process, float vx, float vy, float vz, float vt, float px, float py, float pz, float m, float q, float eta, float phi, float pt, float p, uint32_t vertexPrimaryId, uint32_t vertexSecondaryId, uint32_t particles, uint32_t generation, uint32_t subParticleId);
    ParticleInitial(float vx, float vy, float vz, float px, float py, float pz, float p);

    uint32_t eventId_;
    uint64_t particleId_;
    uint32_t particleType_;
    uint32_t process_;
    float vx_;
    float vy_;
    float vz_;
    float vt_;
    float directionX_;
    float directionY_;
    float directionZ_;
    float px_;
    float py_;
    float pz_;
    float m_;
    float q_;
    float eta_;
    float phi_;
    float pt_;
    float p_;
    uint32_t vertexPrimaryId_;
    uint32_t vertexSecondaryId_;
    uint32_t particles_;
    uint32_t generation_;
    uint32_t subParticleId_;
};
}   // namespace DataTypes
