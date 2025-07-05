#include "DataTypes/Spacepoint.h"

DataTypes::Spacepoint::Spacepoint(uint32_t eventId, uint32_t measurementId, uint32_t geometryId, float x, float y, float z, float varR, float varZ)
: eventId_(eventId), measurementId_(measurementId), geometryId_(geometryId), x_(x), y_(y), z_(z), varR_(varR), varZ_(varZ) {}

DataTypes::Spacepoint::Spacepoint(float x, float y, float z)
: eventId_(0), measurementId_(0), geometryId_(0), x_(x), y_(y), z_(z), varR_(0), varZ_(0) {}
