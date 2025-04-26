#pragma once

#include "EventUsm/DeviceResource.h"
#include "EventUsm/IQueue.h"

#include <queue>
#include <unordered_map>
#include <mutex>

class QueueUsm : public IQueue
{
public:
    QueueUsm(sycl::queue& syclQueue, Capacity resourcesCapacity, Capacity workCapacity);
    ~QueueUsm() override;

    bool createResources(const CreateResourceGroupFunction& createResourceGroupFunction) override;

    inline Capacity getResourcesCapacity() const override;
    inline Capacity getResourcesLoad() const override;
    std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getResources() override;
    void returnResources(DeviceResourceGroupId resourceGroupId) override;

    inline Capacity getWorkCapacity() const override;
    inline Capacity getWorkLoad() const override;
    inline void incrementWorkLoad() override;
    inline void decrementWorkLoad() override;

    inline sycl::queue& checkoutQueue() override;
    inline void checkinQueue() override;
    inline const sycl::queue& getQueue() const override;

    void forEachResourceGroup(const ForEachResourceGroupFunction& callback) override;

    static const DeviceResourceGroup NullResourceGroup; // Used when no resources are available.
    static constexpr DeviceResourceGroupId NullResourceGroupId{0};

protected:
    sycl::queue& syclQueue_;
    std::mutex syclQueueMutex_;

    const Capacity resourcesCapacity_;
    Capacity resourcesLoad_ = 0;
    std::unordered_map<DeviceResourceGroupId, std::unique_ptr<DeviceResourceGroup>> resources_;
    std::queue<DeviceResourceGroupId> freeResources_;

    const Capacity workLoadCapacity_;
    Capacity workLoad_ = 0;
};

inline IQueue::Capacity QueueUsm::getResourcesCapacity() const
{
    return resourcesCapacity_;
}

inline IQueue::Capacity QueueUsm::getResourcesLoad() const
{
    return resourcesLoad_;
}

inline IQueue::Capacity QueueUsm::getWorkCapacity() const
{
    return workLoadCapacity_;
}

inline IQueue::Capacity QueueUsm::getWorkLoad() const
{
    return workLoad_;
}

inline void QueueUsm::incrementWorkLoad()
{
    ++workLoad_;
}

inline void QueueUsm::decrementWorkLoad()
{
    --workLoad_;
}

inline sycl::queue& QueueUsm::checkoutQueue()
{
    syclQueueMutex_.lock();
    return syclQueue_;
}

inline void QueueUsm::checkinQueue()
{
    syclQueueMutex_.unlock();
}

inline const sycl::queue& QueueUsm::getQueue() const
{
    return syclQueue_;
}