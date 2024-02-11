#pragma once

#include <CL/sycl.hpp>


class TransferHeavyKernel
{
    using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
    using FloatBufferWriteAccessor = sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device>;

    public:
    TransferHeavyKernel(FloatBufferReadAccessor input, FloatBufferWriteAccessor output)
        : input(input), output(output) {}

    SYCL_EXTERNAL void operator()(sycl::id<1> idx) const;

private:
    FloatBufferReadAccessor input;
    FloatBufferWriteAccessor output;
};

