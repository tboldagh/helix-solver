#include "ConstSizeVector/ConstSizeVector.h"
#include "UtSyclHelpers/UtSyclHelpers.h"

#include "gtest/gtest.h"
#include <CL/sycl.hpp>

class ConstSizeVectorTestSuite : public testing::Test
{
protected:
    sycl::queue queue;

    static constexpr uint32_t testVector10Size = 10;
    uint32_t vectorSize = 0;
    sycl::buffer<uint32_t, 1> vectorSizeBuffer{&vectorSize, sycl::range<1>(1)};

    std::vector<uint32_t> testVector10{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    sycl::buffer<uint32_t, 1>testVector10Buffer{testVector10.begin(), testVector10.end()};
};


TEST_F(ConstSizeVectorTestSuite, constructFromAccessor)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class ConstructFromAccessorKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, testVector10);
}

TEST_F(ConstSizeVectorTestSuite, copyConstruct)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class CopyConstructKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            ConstSizeVector<uint32_t, 10> constSizeVector2(constSizeVector);

            constSizeVector2.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector2.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, testVector10);
}

TEST_F(ConstSizeVectorTestSuite, assignmentOperator)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class AssignmentOperatorKernel>([=]()
        {
            const ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);
            ConstSizeVector<uint32_t, 10> constSizeVector2;

            constSizeVector2 = constSizeVector;

            constSizeVector2.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector2.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, testVector10);
}

TEST_F(ConstSizeVectorTestSuite, constIndexOperator)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class ConstIndexOperatorKernel>([=]()
        {
            const ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);
            ConstSizeVector<uint32_t, 10> constSizeVector2;

            constSizeVector2.pushBack(constSizeVector[0] + 10);
            constSizeVector2.pushBack(constSizeVector[1] + 10);
            constSizeVector2.pushBack(constSizeVector[2] + 10);

            constSizeVector2.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector2.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 3);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {10, 11, 12, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, indexOperator)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class IndexOperatorKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);
            constSizeVector[0] = 10;
            constSizeVector[1] += 10;
            constSizeVector[2] += 10;

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {10, 11, 12, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, pushBack)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class PushBackKernel>([=](){
            ConstSizeVector<uint32_t, testVector10Size> constSizeVector;

            for(uint32_t i = 0; i < testDataAccessor.size(); ++i)
            {
                constSizeVector.pushBack(testDataAccessor[i]);
            }

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, testVector10);
}

TEST_F(ConstSizeVectorTestSuite, movePushBack)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class MovePushBackKernel>([=](){
            ConstSizeVector<uint32_t, testVector10Size> constSizeVector;

            for(uint32_t i = 0; i < testDataAccessor.size(); ++i)
            {
                constSizeVector.pushBack(std::move(testDataAccessor[i]));
            }

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, testVector10);
}

TEST_F(ConstSizeVectorTestSuite, popBack)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class PopBackKernel>([=](){
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            for(uint32_t i = 0; i < 3; ++i)
            {
                constSizeVector.popBack();
            }
            constSizeVector.pushBack(42);
            constSizeVector.pushBack(43);
            constSizeVector.pushBack(44);


            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {0, 1, 2, 3, 4, 5, 6, 42, 43, 44}));
}

TEST_F(ConstSizeVectorTestSuite, constFront)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class ConstFrontKernel>([=]()
        {
            const ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);
            ConstSizeVector<uint32_t, 10> constSizeVector2;

            constSizeVector2.pushBack(constSizeVector.front());
            constSizeVector2.pushBack(constSizeVector.front());
            constSizeVector2.pushBack(constSizeVector.front());

            constSizeVector2.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector2.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 3);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {0, 0, 0, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, front)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class FrontKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.front() = 42;

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {42, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, constBack)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class ConstBackKernel>([=]()
        {
            const ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);
            ConstSizeVector<uint32_t, 10> constSizeVector2;

            constSizeVector2.pushBack(constSizeVector.back());
            constSizeVector2.pushBack(constSizeVector.back());
            constSizeVector2.pushBack(constSizeVector.back());

            constSizeVector2.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector2.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 3);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {9, 9, 9, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, back)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class BackKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.back() = 42;

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {0, 1, 2, 3, 4, 5, 6, 7, 8, 42}));
}

TEST_F(ConstSizeVectorTestSuite, clear)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class ClearKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.clear();

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 0);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, isEmpty)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class IsEmptyKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.clear();
            if(constSizeVector.isEmpty())
            {
                constSizeVector.pushBack(42);
            }

            if(constSizeVector.isEmpty())
            {
                constSizeVector.pushBack(42);
            }

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 1);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {42, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST_F(ConstSizeVectorTestSuite, getSize)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto vectorSizeAccessor = vectorSizeBuffer.get_access<sycl::access::mode::write>(handler);
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class GetSizeKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector;

            while(constSizeVector.getSize() < constSizeVector.getMaxSize())
            {
                constSizeVector.pushBack(constSizeVector.getSize() + 10);
            }

            constSizeVector.copyToAccessor(testDataAccessor);    
            vectorSizeAccessor[0] = constSizeVector.getSize();
        });
    });
    queue.wait();

    UtSyclHelpers::copyVariableFromBufferToHost<>(vectorSizeBuffer, vectorSize);
    EXPECT_EQ(vectorSize, 10);

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {10, 11, 12, 13, 14, 15, 16, 17, 18, 19}));
}

TEST_F(ConstSizeVectorTestSuite, copyToAccessorCustomRange)
{
    queue.submit([&](sycl::handler& handler)
    {
        auto testDataAccessor = testVector10Buffer.get_access<sycl::access::mode::read_write>(handler);
    
        handler.single_task<class CopyToAccessorCustomRangeKernel>([=]()
        {
            ConstSizeVector<uint32_t, 10> constSizeVector(testDataAccessor);

            constSizeVector.copyToAccessor(testDataAccessor, 4, 8);    
        });
    });
    queue.wait();

    auto testDataAccessor = testVector10Buffer.get_host_access();
    std::vector<uint32_t> newVector10;
    UtSyclHelpers::copyBufferToHostVector(testDataAccessor, newVector10);
    EXPECT_EQ(newVector10, (std::vector<uint32_t> {4, 5, 6, 7, 4, 5, 6, 7, 8, 9}));
}