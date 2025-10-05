#pragma once

#include <sycl/sycl.hpp>


class KernelMemory
{
public:
    KernelMemory(sycl::queue& queue);
    virtual ~KernelMemory() = default;

    KernelMemory& operator=(const KernelMemory&) = delete;

    inline void allocate();
    inline void deallocate();
    inline bool isAllocated() const { return isAllocated_; }
    inline sycl::queue& getQueue() const { return queue_; }

protected:
    virtual void allocateInternal() = 0;
    virtual void deallocateInternal() = 0;

    sycl::queue& queue_;
    bool isAllocated_ = false;
};


inline void KernelMemory::allocate()
{
    if (isAllocated_)
    {
        return;
    }

    allocateInternal();
    isAllocated_ = true;
}

inline void KernelMemory::deallocate()
{
    if (!isAllocated_)
    {
        return;
    }

    deallocateInternal();
    isAllocated_ = false;
}