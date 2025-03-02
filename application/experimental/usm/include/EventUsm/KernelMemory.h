#pragma once

#include <sycl/sycl.hpp>


class KernelMemory
{
public:
    KernelMemory(sycl::queue& queue);
    virtual ~KernelMemory() = default;

    KernelMemory& operator=(const KernelMemory&) = delete;

    virtual void allocateOnDevice() = 0;
    virtual void deallocateOnDevice() = 0;

protected:
    sycl::queue& queue_;
};
