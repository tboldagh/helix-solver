#pragma once
#include <cmath>

// this functions assert whether point belongs to a z-phi-eta slice
// the definition of the region is fiven by:
// phi center,  delta phi,
// z center, delta z - at the beam line (typically unchanged and covering all +-
// 20cm) eta center, delta eta



struct Reg {
  float center;
  float width;
  Reg(float center_val, float width_val) {
    center = center_val;
    width = width_val;
  }
};

/**
 * @brief description of the wedge
 */
struct Wedge {
  /**
   * this is constructor in a form suitable fro SYCL
   */
  Wedge(Reg phi, Reg z, Reg eta) : m_phi(phi) {
    ASSURE_THAT(phi.width > 0.1,
                "Wedge is very narrow in phi < 0.1, not good idea")
    ASSURE_THAT(eta.width > 0.1, "Wedge is very narrow in eta < 0.1, not good "
                                 "idea (also do not support negative widths)")
    // ASSURE_THAT( std::fabs( eta.center + eta.width)> 0.01, "Wedge does not
    // support eta + eta width == 0") ASSURE_THAT( std::fabs( eta.center -
    // eta.width)> 0.01, "Wedge does not support eta - eta width == 0")

    if (std::fabs(eta.center + eta.width) < 0.01) {

      eta.width = 0.01 - eta.center;
    } else if (std::fabs(eta.center - eta.width) < 0.01) {

      eta.width = 0.01 + eta.center;
    }

    m_aleft = std::tan(2.0 * std::atan(std::exp(-(eta.center - eta.width))));
    m_aright = std::tan(2.0 * std::atan(std::exp(-(eta.center + eta.width))));
    m_bleft = -m_aleft / (z.center - z.width);
    m_bright = -m_aright / (z.center + z.width);
  }

  Reg m_phi;

  float phi_min() const { return m_phi.center - m_phi.width; }
  float phi_max() const { return m_phi.center + m_phi.width; }

  // definition of lines in r - z plane
  float m_aleft, m_bleft;
  float m_aright, m_bright;
  /**
   * @brief true if point is in the Wedge
   *
   * @param x,y,z - cartesian coordinates
   */
  bool in_wedge_x_y_z(float x, float y, float z) const {
    return in_wedge_r_phi_z(std::hypot(x, y), std::atan2(y, x), z);
  }
  /**
   * @brief true if point is in the Wedge
   *
   * @param p, phi, z  - polar coordinates
   */
  bool in_wedge_r_phi_z(float r, float phi, float z) const {
    ASSURE_THAT(
        std::fabs(r) > 0.1,
        "Strange r, it needs to be larger than 0.1 (meaning larger than 0)");
    if (std::fabs(phi_dist(phi, m_phi.center)) > m_phi.width)
      return false;

    // 3 cases
    if (m_aleft > 0 &&
        m_aright > 0) { // both left & right are  right tilted like this / /
      // std::cout << " here r" << r << " z " << z << " " << m_aleft * z +
      // m_bleft
      // << " " <<  m_aright * z + m_bright << "\n";
      return m_aleft * z + m_bleft > r && r > m_aright * z + m_bright;
    } else if (m_aleft < 0 && m_aright > 0) { // this lind of wedge \ /
      return m_aleft * z + m_bleft < r && r > m_aright * z + m_bright;
    } else { // left tilt situation
      return m_aleft * z + m_bleft < r && r < m_aright * z + m_bright;
    }
    ASSURE_THAT(
        true,
        "Strange position of the point, does not fit any wedge definition");
    return false;
  }

  /**
  * uniform split
  */
  static Reg uniform_split(float min, float max, short index, short splits) {
    float width = (max - min) / splits;
    return {min + width * index + 0.5f * width, 0.5f * width};
  }

  static float phi_wrap(float phi) {
    while (phi > M_PI) {
      phi -= 2.0 * M_PI;
    }
    while (phi <= -M_PI) {
      phi += 2.0 * M_PI;
    }
    return phi;
  }

  static float phi_dist(float phi1, float phi2) {
    float dphi = phi1 - phi2;
    return phi_wrap(dphi);
  }



  /**
   * @brief returns true if the the point given as an argument is below top line
   */
  bool below(float r, float z) const;
  /**
   * @brief returns true if the the point given as an argument is above bottom
   * line
   */
  bool above(float r, float z) const;
};
