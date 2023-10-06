#include "HelixSolver/AccumulatorSection.h"
// #include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace HelixSolver {

class SubDivTestSuite : public ::testing::Test {
protected:
  void SetUp() override { 
    m_10x10 = AccumulatorSection(10, 10, 2, 2, 0); // 10 x 10 rooted at (2,2)
    m_8x10 = AccumulatorSection(8, 10, 0, 2, 0);   // 8 x 18  rooted at (0, 2)
    } 

  void TearDown() override {}
  AccumulatorSection m_10x10;
  AccumulatorSection m_8x10;

};

TEST_F(SubDivTestSuite, UnofrimSplit) {
  ASSERT_FLOAT_EQ(m_10x10.bottomRight().xSize, 5);
  ASSERT_FLOAT_EQ(m_10x10.bottomRight().ySize, 5);

  ASSERT_FLOAT_EQ(m_10x10.topRight().xSize, 5);
  ASSERT_FLOAT_EQ(m_10x10.topRight().ySize, 5);
  ASSERT_FLOAT_EQ(m_10x10.bottomLeft().xSize, 5);
  ASSERT_FLOAT_EQ(m_10x10.bottomLeft().ySize, 5);
  ASSERT_FLOAT_EQ(m_10x10.topLeft().xSize, 5);
  ASSERT_FLOAT_EQ(m_10x10.topLeft().ySize, 5);
}

TEST_F(SubDivTestSuite, NonUniformSplit) {
  auto br = m_10x10.bottomRight(0.6, 0.7);
  ASSERT_FLOAT_EQ(br.xSize, 6);
  ASSERT_FLOAT_EQ(br.ySize, 7);
  ASSERT_FLOAT_EQ(br.xBegin, 6);
  ASSERT_FLOAT_EQ(br.yBegin, 2);

  auto bl = m_10x10.bottomLeft(0.6, 0.8);
  ASSERT_FLOAT_EQ(bl.xSize, 6);
  ASSERT_FLOAT_EQ(bl.ySize, 8);
  ASSERT_FLOAT_EQ(bl.xBegin, 2);
  ASSERT_FLOAT_EQ(bl.yBegin, 2);

  auto tl = m_10x10.topLeft(0.6, 0.8);
  ASSERT_FLOAT_EQ(tl.xSize, 6);
  ASSERT_FLOAT_EQ(tl.ySize, 8);
  ASSERT_FLOAT_EQ(tl.xBegin, 2);
  ASSERT_FLOAT_EQ(tl.yBegin, 4);


  auto tr = m_10x10.topRight(0.6, 0.8);
  ASSERT_FLOAT_EQ(tr.xSize, 6);
  ASSERT_FLOAT_EQ(tr.ySize, 8);
  ASSERT_FLOAT_EQ(tr.xBegin, 6);
  ASSERT_FLOAT_EQ(tr.yBegin, 4);
}

TEST_F(SubDivTestSuite, Halves) {
  auto t = m_10x10.top(0.6);
  ASSERT_FLOAT_EQ(t.xSize, 10);
  ASSERT_FLOAT_EQ(t.ySize, 6);
  ASSERT_FLOAT_EQ(t.xBegin, 2);
  ASSERT_FLOAT_EQ(t.yBegin, 6);

  auto b = m_10x10.bottom(0.6);
  ASSERT_FLOAT_EQ(b.xSize, 10);
  ASSERT_FLOAT_EQ(b.ySize, 6);
  ASSERT_FLOAT_EQ(b.xBegin, 2);
  ASSERT_FLOAT_EQ(b.yBegin, 2);

  auto l = m_10x10.left(0.6);
  ASSERT_FLOAT_EQ(l.xSize, 6);
  ASSERT_FLOAT_EQ(l.ySize, 10);
  ASSERT_FLOAT_EQ(l.xBegin, 2);
  ASSERT_FLOAT_EQ(l.yBegin, 2);

  auto r = m_10x10.right(0.6);
  ASSERT_FLOAT_EQ(r.xSize, 6);
  ASSERT_FLOAT_EQ(r.ySize, 10);
  ASSERT_FLOAT_EQ(r.xBegin, 6);
  ASSERT_FLOAT_EQ(r.yBegin, 2);

}

TEST_F(SubDivTestSuite, DistClockWise) {
  // check counter clockwise from upper left section corner
  //       /      /
  // +----/------+
  //     /      /|
  //    a)    b) |/
  //             /
  //            /|
  //           c)

  // a)
  ASSERT_FLOAT_EQ(m_10x10.distCC(0.5, 8), 6); // 8 - 2 (the later is because section is shifted by 2)
  ASSERT_FLOAT_EQ(m_8x10.distCC(0.5, 10), 4); // (the later is because section is shifted by 2)

  // b) - this is trivial case
  ASSERT_FLOAT_EQ(m_10x10.distCC(1, 0), m_10x10.xSize); // line going through the top right kernel
  ASSERT_FLOAT_EQ(m_8x10.distCC(1, 4), m_8x10.xSize); // line going through the top right kernel

  // c)
  ASSERT_FLOAT_EQ(m_10x10.distCC(1, -2), 12);
  ASSERT_FLOAT_EQ(m_8x10.distCC(1, -2), 14); 
}


TEST_F(SubDivTestSuite, DistAntiClockWise) {
  // check counter clockwise from upper left section corner
  //    +
  //    |
  //    |/
  //    /
  //   /| /   /
  //  a)+----/-----
  //   /    /
  // b)    c)

  // a)
  ASSERT_FLOAT_EQ(m_10x10.distACC(0.5, 8), 3); // 8 - 2 (the later is because section is shifted by 2)
  ASSERT_FLOAT_EQ(m_8x10.distACC(0.5, 8), 4); // (the later is because section is shifted by 2)

  // b) - this is trivial case
  ASSERT_FLOAT_EQ(m_10x10.distACC(1, 0), m_10x10.ySize); // line going through bottom left
  ASSERT_FLOAT_EQ(m_8x10.distACC(1, 2), m_8x10.ySize); // line going through bottom left
  ASSERT_FLOAT_EQ(m_8x10.distACC(0.33, 2), m_8x10.ySize); // line going through bottom left (the slope does not matter)

  // c)
  ASSERT_FLOAT_EQ(m_10x10.distACC(1, -2), 12);
  ASSERT_FLOAT_EQ(m_8x10.distACC(1, -2), 14); 
  ASSERT_FLOAT_EQ(m_8x10.distACC(0.5, -2), 18); 
  ASSERT_FLOAT_EQ(m_8x10.distACC(0.5, -1), 16); 
  ASSERT_FLOAT_EQ(m_8x10.distACC(0.5, 1), 12); 

}


} // namespace HelixSolver