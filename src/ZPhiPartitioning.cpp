#include "HelixSolver/ZPhiPartitioning.h"
#include <stdexcept>
#include <iostream>

#include "HelixSolver/Debug.h"


float phi_wrap( float phi ) {
  while ( phi > M_PI ) {
    phi -= 2.0*M_PI;
  }
  while ( phi <= -M_PI ) {
    phi += 2.0*M_PI;
  }
  return phi;
}

float phi_dist ( float phi1, float phi2 ) {
  float dphi  = phi1 - phi2;
  return phi_wrap(dphi);
}

Reg::Reg(float center_val, float width_val){

  center = center_val;
  width = width_val;
}


Wedge::Wedge( Reg p, Reg z, Reg eta)
    : m_phi(p) {
    ASSURE_THAT( p.width> 0.1, "Wedge is very narrow in phi < 0.1, not good idea")
    ASSURE_THAT( eta.width > 0.1, "Wedge is very narrow in eta < 0.1, not good idea (also do not support negative widths)")
    //ASSURE_THAT( std::fabs( eta.center + eta.width)> 0.01, "Wedge does not support eta + eta width == 0")
    //ASSURE_THAT( std::fabs( eta.center - eta.width)> 0.01, "Wedge does not support eta - eta width == 0")

    if (std::fabs(eta.center + eta.width) < 0.01){

      eta.width = 0.01 - eta.center;
    } else if (std::fabs(eta.center - eta.width) < 0.01){

      eta.width = 0.01 + eta.center;
    }

    m_aleft = std::tan( 2.0 * std::atan( std::exp( - (eta.center-eta.width))));
    m_aright = std::tan( 2.0 * std::atan( std::exp( - (eta.center+eta.width))));
    m_bleft = - m_aleft/(z.center - z.width);
    m_bright = - m_aright/(z.center + z.width);
}

bool Wedge::in_wedge_x_y_z( float x, float y, float z ) const {
    return in_wedge_r_phi_z(std::hypot(x,y), std::atan2(y, x), z);
}

bool Wedge::in_wedge_r_phi_z( float r, float phi, float z ) const {
    ASSURE_THAT( std::fabs(r) > 0.1, "Strange r, it needs to be larger than 0.1 (meaning larger than 0)");
    if ( std::fabs(phi_dist(phi, m_phi.center)) > m_phi.width) return false;

    // 3 cases
    if ( m_aleft > 0 && m_aright > 0  ) { //both left & right are  right tilted like this / /
        // std::cout << " here r" << r << " z " << z << " " << m_aleft * z + m_bleft << " " <<  m_aright * z + m_bright << "\n";
        return m_aleft * z + m_bleft > r && r > m_aright * z + m_bright;
    } else if ( m_aleft < 0 && m_aright > 0 ) { // this lind of wedge \ /
        return m_aleft * z + m_bleft < r && r > m_aright * z + m_bright;
    } else { // left tilt situation
        return m_aleft * z + m_bleft < r && r < m_aright * z + m_bright;
    }
    ASSURE_THAT( true, "Strange position of the point, does not fit any wedge definition");
    return false;

}