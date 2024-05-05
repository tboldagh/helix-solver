#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"


EventUsm::EventUsm(EventId eventId)
: eventId_(eventId) {}

EventUsm::~EventUsm()
{
    if (allocated_)
    {
        LOG_ERROR("Memory leak in EventUsm with eventId " + std::to_string(eventId_) + ". Memory was not deallocated on device before destruction.");
    }
}

bool EventUsm::allocateOnDevice(sycl::queue& queue)
{
    if (allocated_)
    {
        LOG_ERROR("Memory already allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        deviceNumPoints_ = sycl::malloc_device<u_int32_t>(1, queue);
        deviceXs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceYs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceZs_ = sycl::malloc_device<float>(MaxPoints, queue);
        deviceLayers_ = sycl::malloc_device<LayerNumber>(MaxPoints, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to allocate memory on device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = true;
    allocationQueue_ = &queue;
    return true;
}

bool EventUsm::deallocateOnDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return false;
    }

    try
    {
        sycl::free(deviceNumPoints_, queue);
        sycl::free(deviceXs_, queue);
        sycl::free(deviceYs_, queue);
        sycl::free(deviceZs_, queue);
        sycl::free(deviceLayers_, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to deallocate memory on device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = false;
    allocationQueue_ = nullptr;
    return true;
}

DataUsm::TransferEvents EventUsm::transferToDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return TransferEvents{};
    }

    if (allocationQueue_ != &queue)
    {
        LOG_ERROR("Memory allocated on different queue for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceNumPoints_, &hostNumPoints_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceXs_, hostXs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceYs_, hostYs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceZs_, hostZs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceLayers_, hostLayers_, hostNumPoints_ * sizeof(LayerNumber))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to device for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return TransferEvents{};
    }

    return transferEvents;
}

DataUsm::TransferEvents EventUsm::transferToHost(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return TransferEvents{};
    }

    if (allocationQueue_ != &queue)
    {
        LOG_ERROR("Memory allocated on different queue for EventUsm with eventId " + std::to_string(eventId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(&hostNumPoints_, deviceNumPoints_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostXs_, deviceXs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostYs_, deviceYs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostZs_, deviceZs_, hostNumPoints_ * sizeof(float))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostLayers_, deviceLayers_, hostNumPoints_ * sizeof(LayerNumber))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer memory to host for EventUsm with eventId " + std::to_string(eventId_) + ". Exception: " + exception.what() + ".");
        return TransferEvents{};
    }

    return transferEvents;
}

bool EventUsm::takeResourceGroup(const DeviceResourceGroup& resourceGroup, const sycl::queue& queue)
{
    if (allocated_)
    {
        LOG_WARNING("Memory already allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ". Not taking ownership of resource group.");
        return false;
    }

    deviceNumPoints_ = static_cast<u_int32_t*>(resourceGroup.at(DeviceResourceType::NumPoints));
    deviceXs_ = static_cast<float*>(resourceGroup.at(DeviceResourceType::Xs));
    deviceYs_ = static_cast<float*>(resourceGroup.at(DeviceResourceType::Ys));
    deviceZs_ = static_cast<float*>(resourceGroup.at(DeviceResourceType::Zs));
    deviceLayers_ = static_cast<LayerNumber*>(resourceGroup.at(DeviceResourceType::Layers));

    resourcesBorrowed_ = true;
    allocationQueue_ = &queue;

    return true;
}

std::pair<std::unique_ptr<DeviceResourceGroup>, const sycl::queue*> EventUsm::releaseResourceGroup()
{
    if (allocated_)
    {
        LOG_WARNING("Memory not allocated on device for EventUsm with eventId " + std::to_string(eventId_) + ". Not releasing resource group.");
        return {nullptr, nullptr};
    }

    auto resourceGroup = std::make_unique<DeviceResourceGroup>();
    resourceGroup->emplace(DeviceResourceType::NumPoints, deviceNumPoints_);
    resourceGroup->emplace(DeviceResourceType::Xs, deviceXs_);
    resourceGroup->emplace(DeviceResourceType::Ys, deviceYs_);
    resourceGroup->emplace(DeviceResourceType::Zs, deviceZs_);
    resourceGroup->emplace(DeviceResourceType::Layers, deviceLayers_);

    resourcesBorrowed_ = false;
    allocationQueue_ = nullptr;

    return {std::move(resourceGroup), allocationQueue_};
}

std::unique_ptr<DeviceResourceGroup> EventUsm::allocateDeviceResources(sycl::queue& queue)
{
    auto resourceGroup = std::make_unique<DeviceResourceGroup>();
    resourceGroup->emplace(DeviceResourceType::NumPoints, sycl::malloc_device<u_int32_t>(1, queue));
    resourceGroup->emplace(DeviceResourceType::Xs, sycl::malloc_device<float>(MaxPoints, queue));
    resourceGroup->emplace(DeviceResourceType::Ys, sycl::malloc_device<float>(MaxPoints, queue));
    resourceGroup->emplace(DeviceResourceType::Zs, sycl::malloc_device<float>(MaxPoints, queue));
    resourceGroup->emplace(DeviceResourceType::Layers, sycl::malloc_device<LayerNumber>(MaxPoints, queue));

    return resourceGroup;
}

void EventUsm::deallocateDeviceResources(DeviceResourceGroup& resourceGroup, sycl::queue& queue)
{
    sycl::free(resourceGroup.at(DeviceResourceType::NumPoints), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Xs), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Ys), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Zs), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Layers), queue);
}
