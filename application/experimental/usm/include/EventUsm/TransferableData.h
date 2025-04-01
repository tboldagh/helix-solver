#pragma once

#include "EventUsm/DeviceResource.h"
#include "EventUsm/KernelMemory.h"

#include <sycl/sycl.hpp>
#include <memory>
#include <unordered_set>


class TransferableData
{
public:
    using TransferEvents = std::unordered_set<std::unique_ptr<sycl::event>>;

    TransferableData() = default;
    virtual ~TransferableData() = default;

    TransferableData(const TransferableData&) = delete;

    void setKernelMemory(KernelMemory* kernelMemory);
    inline bool isKernelMemorySet() const { return kernelMemorySet_; }
    virtual TransferEvents transferToDevice() = 0;
    virtual TransferEvents transferToHost() = 0;

protected:
    // kernelMemory == nullptr to unset
    virtual void setKernelMemoryInternal(KernelMemory* kernelMemory) = 0;

    bool kernelMemorySet_ = false;
};
