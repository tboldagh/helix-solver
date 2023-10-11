#include "HelixSolver/Sorting.h"
#include "gtest/gtest.h"

namespace HelixSolver {

class SorterTestSuite : public ::testing::Test {
};

TEST_F(SorterTestSuite, NoSort) {
    float d[] = {1,2,4};
    uint32_t i[] = {9,1,2};

    CrossingsSorter::sort( d, i, 3);

    ASSERT_EQ(i[0], 9);
    ASSERT_EQ(i[1], 1);
    ASSERT_EQ(i[2], 2);
}

TEST_F(SorterTestSuite, Real) {
    float d[] = {4,1,2};
    uint32_t i[] = {9,1,2};

    CrossingsSorter::sort( d, i, 3);
    ASSERT_EQ(i[0], 1);
    ASSERT_EQ(i[1], 2);
    ASSERT_EQ(i[2], 9);
}


TEST_F(SorterTestSuite, RealLonger) {
    float d[] = {4,1,2,5,3,2};
    uint32_t i[] = {9,1,2,0,12,100};

    CrossingsSorter::sort( d, i, 6);
    ASSERT_EQ(i[0], 1);
    ASSERT_EQ(i[1], 2);
    ASSERT_EQ(i[2], 100);
    ASSERT_EQ(i[3], 12);
    ASSERT_EQ(i[4], 9);
    ASSERT_EQ(i[5], 0);

}




}