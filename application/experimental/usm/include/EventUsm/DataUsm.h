#pragma once

#include "EventUsm/DeviceResource.h"

#include <CL/sycl.hpp>
#include <memory>
#include <unordered_set>


class DataUsm
{
public:
    using TransferEvents = std::unordered_set<std::unique_ptr<sycl::event>>;

    virtual bool allocateOnDevice(sycl::queue& queue) = 0;
    virtual bool deallocateOnDevice(sycl::queue& queue) = 0;
    // Returns sycl::event set. Each events needs to be waited on before using the data.
    virtual TransferEvents transferToDevice(sycl::queue& queue) = 0;
    virtual TransferEvents transferToHost(sycl::queue& queue) = 0;

    inline bool isAllocated() const;
    inline bool resourcesBorrowed() const;
    // Returns false when allocated. Does not take ownership of the resource group when failed.
    virtual bool takeResourceGroup(const DeviceResourceGroup& resourceGroup, const sycl::queue& queue) = 0;
    // Queue is nullptr and resource group is empty when resources are not allocated.
    virtual std::pair<std::unique_ptr<DeviceResourceGroup>, const sycl::queue*> releaseResourceGroup() = 0;

protected:
    bool allocated_ = false;
    bool resourcesBorrowed_ = false;
    const sycl::queue* allocationQueue_ = nullptr;    // The queue used for last allocation;
};

inline bool DataUsm::isAllocated() const
{
    return allocated_;
}

inline bool DataUsm::resourcesBorrowed() const
{
    return resourcesBorrowed_;
}