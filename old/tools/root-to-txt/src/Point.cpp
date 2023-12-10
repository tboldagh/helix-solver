#include "Point.h"

Point::Point(double p_x, double p_y, double p_z, uint8_t p_layer) :
        m_x(p_x), m_y(p_y), m_z(p_z), m_layer(p_layer) {}

double Point::getX() const {
    return m_x;
}

double Point::getY() const {
    return m_y;
}

double Point::getZ() const {
    return m_z;
}

uint8_t Point::getLayer() const {
    return m_layer;
}