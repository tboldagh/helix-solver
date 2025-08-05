#include "SpacepointsGenerator/SpacepointsGenerator.h"

#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdint>


SpacepointsGenerator::SpacepointsGenerator(float maxAbsZ, float maxAbsXy)
: maxAbsZ_(maxAbsZ), maxAbsXy_(maxAbsXy) {}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const float xAngle, const float zAngle, const float interactionZ, const float r, const bool counterClockwise, const uint8_t numPoints) const
{
    float phi = zAngle + 0.5f * M_PI;
    float bendDirection = counterClockwise ? 1.0f : -1.0f;

    float directionZ = std::cos(xAngle);
    float directionXy = std::sin(xAngle);

    float scale = std::min(std::abs(maxAbsXy_ / std::max(std::abs(directionXy), 1e-6f)), std::abs((maxAbsZ_ - interactionZ) / std::max(std::abs(directionZ), 1e-6f)));
    float farZ = directionZ * scale + interactionZ;
    float farXy = directionXy * scale;
    float farAlpha = farXy / r * bendDirection;

    float centerX = r * std::cos(phi) * bendDirection;
    float centerY = r * std::sin(phi) * bendDirection;

    output.reserve(output.size() + numPoints);

    for (uint8_t i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i + 1) / numPoints;

        float alpha = lerp(0, farAlpha, t);
        float z = lerp(interactionZ, farZ, t);
        
        float x = (-centerX * std::cos(alpha) - (-centerY * std::sin(alpha))) + centerX;
        float y = -centerY * std::cos(alpha) + (-centerX * std::sin(alpha)) + centerY;

        output.emplace_back(x, y, z);
    }
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const float xAngle, const float zAngle, const float interactionZ, const float r, const bool counterClockwise, const uint8_t numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, xAngle, zAngle, interactionZ, r, counterClockwise, numPoints);
    return output;
}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints) const
{
    output.reserve(output.size() + sumVector(numPoints));
    for (uint8_t i = 0; i < xAngles.size(); ++i)
    {
        generate(output, xAngles[i], zAngles[i], interactionZs[i], rs[i], counterClockwise[i], numPoints[i]);
    }
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const std::vector<float>& xAngles, const std::vector<float>& zAngles, const std::vector<float>& interactionZs, const std::vector<float>& rs, const std::vector<bool>& counterClockwise, const std::vector<uint8_t>& numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, xAngles, zAngles, interactionZs, rs, counterClockwise, numPoints);
    return output;
}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const DataTypes::ParticleInitial& particleInitial, const float r, const bool counterClockwise, const uint8_t numPoints) const
{
    const auto [xAngle, zAngle] = directionToAngles(particleInitial.directionX_, particleInitial.directionY_, particleInitial.directionZ_);
    generate(output, xAngle, zAngle, particleInitial.vz_, r, counterClockwise, numPoints);
}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const DataTypes::ParticleInitial& particleInitial, const float r, const uint8_t numPoints) const
{
    const bool counterClockwise = particleInitial.directionX_ < 0;
    generate(output, particleInitial, r, counterClockwise, numPoints);
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const DataTypes::ParticleInitial& particleInitial, const float r, const bool counterClockwise, const uint8_t numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, particleInitial, r, counterClockwise, numPoints);
    return output;
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const DataTypes::ParticleInitial& particleInitial, const float r, const uint8_t numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, particleInitial, r, numPoints);
    return output;
}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<bool>& counterClockwises, const std::vector<uint8_t>& numPoints) const
{
    output.reserve(output.size() + sumVector(numPoints));
    for (uint8_t i = 0; i < particleInitials.size(); ++i)
    {
        generate(output, particleInitials[i], rs[i], counterClockwises[i], numPoints[i]);
    }
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<bool>& counterClockwises, const std::vector<uint8_t>& numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, particleInitials, rs, counterClockwises, numPoints);
    return output;
}

void SpacepointsGenerator::generate(std::vector<DataTypes::Spacepoint>& output, const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<uint8_t>& numPoints) const
{
    output.reserve(output.size() + sumVector(numPoints));
    for (uint8_t i = 0; i < particleInitials.size(); ++i)
    {
        generate(output, particleInitials[i], rs[i], numPoints[i]);
    }
}

std::vector<DataTypes::Spacepoint> SpacepointsGenerator::generate(const std::vector<DataTypes::ParticleInitial>& particleInitials, const std::vector<float>& rs, const std::vector<uint8_t>& numPoints) const
{
    std::vector<DataTypes::Spacepoint> output;
    generate(output, particleInitials, rs, numPoints);
    return output;
}

float SpacepointsGenerator::lerp(const float minValue, const float maxValue, const float t)
{
    return minValue + t * (maxValue - minValue);
}

std::pair<float, float> SpacepointsGenerator::directionToAngles(const float directionX, const float directionY, const float directionZ)
{
    float xAngle = std::atan2(std::sqrt(directionY * directionY + directionX * directionX), directionZ);
    float zAngle = angleWrap2Pi(std::atan2(directionY, directionX));
    return std::make_pair(xAngle, zAngle);
}

uint32_t SpacepointsGenerator::sumVector(const std::vector<uint8_t>& vector)
{
    uint32_t sum = 0;
    for (const auto& value : vector)
    {
        sum += value;
    }
    return sum;
}

float SpacepointsGenerator::angleWrap2Pi(const float angle)
{
    return angle - 2.0f * M_PI * std::floor(angle / (2.0f * M_PI));
}