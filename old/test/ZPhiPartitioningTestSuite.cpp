#include "HelixSolver/ZPhiPartitioning.h"
// #include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace HelixSolver {



TEST(PhiSuite, WedgePosPhiTest) {
  Wedge wedge(Reg({1, 0.11}), Reg({0, 20}), Reg({0, 4}) );
  // std::cout << "phi " << wedge.m_phi.center << " "  << wedge.m_phi.width << std::endl;
  // std::cout << "left " << wedge.m_aleft << " "  << wedge.m_bleft << std::endl;
  // std::cout << "right " << wedge.m_aright << " "  << wedge.m_bright << std::endl;
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 1, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 1.1, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 0.9, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 1.12, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 0.88, 0));
}

TEST(PhiSuite, WedgeNegPhiTest) {
  Wedge wedge(Reg({-1, 0.11}), Reg({0, 20}), Reg({0, 4}) );
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -1, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -1.1, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -0.9, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, -1.12, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, -0.88, 0));
}



TEST(PhiSuite, PhiWrappingAround0) {
  Wedge wedge(Reg({0, 0.11}), Reg({0, 20}), Reg({0, 4}) );
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 0, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 0.1, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -0.1, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 0.12, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, -0.12, 0));
}

TEST(PhiSuite, PhiWrappingAroundPI) {
  Wedge wedge(Reg({3.1415, 0.1}), Reg({0, 20}), Reg({0, 4}) );
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 3.14, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -3.14, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 3, 0));
  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, -3, 0));
}


// TEST(RZSuite, RightTilted) {
//   Wedge wedge(Reg({0, 3.1415}), Reg({0, 20}), Reg({3, 1}) );
//   std::cout << "Right\n";
//   std::cout << "phi " << wedge.m_phi.center << " "  << wedge.m_phi.width << std::endl;
//   std::cout << "left " << wedge.m_aleft << " "  << wedge.m_bleft << std::endl;
//   std::cout << "right " << wedge.m_aright << " "  << wedge.m_bright << std::endl;

//   ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 1, 20));
//   // ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 1, 1000));
//   ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 1, -20));
// }


TEST(RZSuite, LeftTilted) {
  Wedge wedge(Reg({0, 3.1415}), Reg({0, 20}), Reg({-3, 1}) );
  std::cout << "Left\n";
  std::cout << "phi " << wedge.m_phi.center << " "  << wedge.m_phi.width << std::endl;
  std::cout << "left " << wedge.m_aleft << " "  << wedge.m_bleft << std::endl;
  std::cout << "right " << wedge.m_aright << " "  << wedge.m_bright << std::endl;

  ASSERT_FALSE(wedge.in_wedge_r_phi_z(1, 3.14, 20));
  // ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, -3.14, 0));
  ASSERT_TRUE(wedge.in_wedge_r_phi_z(1, 3, -20));
}



TEST(RZSuite, Gen) {
  // USED TO GENERATE VISUALISATION
  // Wedge wedge_left(Reg({0, 3.1415}), Reg({0, 20}), Reg({-3, 1}) );
  // Wedge wedge_center(Reg({0, 3.1415}), Reg({0, 20}), Reg({0, 1}) );
  // Wedge wedge_right(Reg({0, 3.1415}), Reg({0, 20}), Reg({3, 1}) );

  // for ( auto r = 10; r < 1000; r += 1 ) {
  //   for ( auto z = -1000; z < 1000; z += 1 ) {
  //     std::cout << z << "," << r << 
  //               "," << wedge_left.in_wedge_r_phi_z(r, 0, z ) <<
  //               "," << wedge_center.in_wedge_r_phi_z(r, 0, z ) <<
  //               "," << wedge_right.in_wedge_r_phi_z(r, 0, z ) << "\n";
  //   }
  // }
}





} // namespace HelixSolver