#pragma once
#include <cmath>

// this functions assert whether point belongs to a z-phi-eta slice
// the definition of the region is fiven by:
// phi center,  delta phi, 
// z center, delta z - at the beam line (typically unchanged and covering all +- 20cm)
// eta center, delta eta



struct Reg { 
    float center;
    float width;
};

/**
 * @brief description of the wedge
 */
struct Wedge {
    Wedge( Reg Phi, Reg z, Reg eta);
    Reg m_phi;
    // definition of lines in r - z plane
    float m_aleft, m_bleft;
    float m_aright, m_bright;
    /**
     * @brief true if point is in the Wedge
     * 
     * @param x,y,z - cartesian coordinates
     */
    bool in_wedge_x_y_z( float x, float y, float z ) const;
    /**
     * @brief true if point is in the Wedge
     * 
     * @param p, phi, z  - polar coordinates
     */
    bool in_wedge_r_phi_z( float r, float phi, float z ) const;

    /**
     * @brief returns true if the the point given as an argument is below top line
     */
    bool below(float r, float z) const;
    /**
     * @brief returns true if the the point given as an argument is above bottom line
     */
    bool above(float r, float z) const;
};

Reg region(float min, float max, uint8_t index, uint8_t splits );

