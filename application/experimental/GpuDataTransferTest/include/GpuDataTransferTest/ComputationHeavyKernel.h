#pragma once

#include <CL/sycl.hpp>


template <uint64_t Iterations>
class ComputationHeavyKernel
{
    using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
    using FloatBufferWriteAccessor = sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device>;

public:
    explicit ComputationHeavyKernel(FloatBufferReadAccessor input, FloatBufferWriteAccessor output)
        : input(input), output(output) {}

    void operator()(sycl::id<1> idx) const;

private:
    FloatBufferReadAccessor input;
    FloatBufferWriteAccessor output;
};

template <uint64_t Iterations>
void ComputationHeavyKernel<Iterations>::operator()(sycl::id<1> idx) const
{
    int index = idx[0];
    float result = 0;
    for (u_int64_t i = 0; i < Iterations; ++i)
    {
        result += input[index] * std::sin(static_cast<float>(index * i));
    }
    output[index] = result;
}