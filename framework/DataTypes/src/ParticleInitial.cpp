#include "DataTypes/ParticleInitial.h"

DataTypes::ParticleInitial::ParticleInitial(uint32_t eventId, uint64_t particleId, uint32_t particleType, uint32_t process, float vx, float vy, float vz, float vt, float px, float py, float pz, float m, float q, float eta, float phi, float pt, float p, uint32_t vertexPrimaryId, uint32_t vertexSecondaryId, uint32_t particles, uint32_t generation, uint32_t subParticleId)
: eventId_(eventId), particleId_(particleId), particleType_(particleType), process_(process), vx_(vx), vy_(vy), vz_(vz), vt_(vt), directionX_(px / p), directionY_(py / p), directionZ_(pz / p), px_(px), py_(py), pz_(pz), m_(m), q_(q), eta_(eta), phi_(phi), pt_(pt), p_(p), vertexPrimaryId_(vertexPrimaryId), vertexSecondaryId_(vertexSecondaryId), particles_(particles), generation_(generation), subParticleId_(subParticleId) {}

DataTypes::ParticleInitial::ParticleInitial(float vx, float vy, float vz, float px, float py, float pz, float p)
: eventId_(0), particleId_(0), particleType_(0), process_(0), vx_(vx), vy_(vy), vz_(vz), vt_(0), directionX_(px / p), directionY_(py / p), directionZ_(pz / p), px_(px), py_(py), pz_(pz), m_(0), q_(0), eta_(0), phi_(0), pt_(0), p_(p), vertexPrimaryId_(0), vertexSecondaryId_(0), particles_(0), generation_(0), subParticleId_(0) {}
