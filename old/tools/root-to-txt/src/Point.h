#pragma once

#include <cstdint>

class Point {
public:
    Point(double p_x, double p_y, double p_z, uint8_t p_layer);

    double getX() const;

    double getY() const;

    double getZ() const;

    uint8_t getLayer() const;

private:
    const double m_x;
    const double m_y;
    const double m_z;
    const uint8_t m_layer;
};
