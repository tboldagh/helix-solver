#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <boost/circular_buffer.hpp>

namespace HelixSolver
{

constexpr int TEST_VECTOR_1[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
constexpr int TEST_VECTOR_2[]{1, 2, 3, 4};

class CircularBufferTestSuite : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_sut = std::make_unique<boost::circular_buffer<int>>(m_sutLen);
    }

    void TearDown() override
    {

    }

    size_t m_sutLen{5};
    std::unique_ptr<boost::circular_buffer<int>> m_sut{nullptr};
};

TEST_F(CircularBufferTestSuite, ShouldStoreAllValuesIfLessPushesThanSizeOfBuffer)
{
    for (int val : TEST_VECTOR_2)
    {
        m_sut->push_back(val);
    }
    
    ASSERT_THAT(*m_sut, ::testing::ElementsAreArray(TEST_VECTOR_2));
}

TEST_F(CircularBufferTestSuite, ShouldStoreOnlyLastValuesIfMorePushesThanSizeOfBuffer)
{
    constexpr int EXPECTED_BUFFER[]{9, 10, 11, 12, 13};
    for (int val : TEST_VECTOR_1)
    {
        m_sut->push_back(val);
    }

    ASSERT_THAT(*m_sut, ::testing::ElementsAreArray(EXPECTED_BUFFER));
}

} // HelixSolver
