#pragma once

#include "DataTypes/Spacepoint.h"
#include "DataTypes/ParticleInitial.h"

#include <vector>
#include <utility>


class SpacepointsGenerator
{
public:
    SpacepointsGenerator(float maxAbsZ, float maxAbsXy);

    void generate(std::vector<DataTypes::Spacepoint>& output, const float xAngle, const float zAngle, const float interactionZ, const float r, const bool counterClockwise, const uint8_t numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const float xAngle, const float zAngle, const float interactionZ, const float r, const bool counterClockwise, const uint8_t numPoints) const;
    
    void generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints) const;

    void generate(std::vector<DataTypes::Spacepoint>& output, const DataTypes::ParticleInitial& particleInitial, const float r, const bool counterClockwise, const uint8_t numPoints) const;
    void generate(std::vector<DataTypes::Spacepoint>& output, const DataTypes::ParticleInitial& particleInitial, const float r, const uint8_t numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const DataTypes::ParticleInitial& particleInitial, const float r, const bool counterClockwise, const uint8_t numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const DataTypes::ParticleInitial& particleInitial, const float r, const uint8_t numPoints) const;

    void generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<bool>& counterClockwises, const std::vector<uint8_t>& numPoints) const;
    void generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<uint8_t>& numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<bool>& counterClockwises, const std::vector<uint8_t>& numPoints) const;
    std::vector<DataTypes::Spacepoint> generate(const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<uint8_t>& numPoints) const;

    static std::pair<float, float> directionToAngles(const float directionX, const float directionY, const float directionZ);
private:
    float maxAbsZ_;
    float maxAbsXy_;

    static float lerp(const float minValue, const float maxValue, const float t);
    static uint32_t sumVector(const std::vector<uint8_t>& vector);
    static float angleWrap2Pi(const float angle);
};
