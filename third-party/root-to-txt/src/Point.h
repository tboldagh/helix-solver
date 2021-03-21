#pragma once

class Point {
public:
    Point(double p_x, double p_y, double p_z);

    double getX() const;
    double getY() const;
    double getZ() const;

private:
    const double m_x;
    const double m_y;
    const double m_z;
};
