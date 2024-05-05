#include "EventUsm/ResultUsm.h"
#include "Logger/Logger.h"


ResultUsm::ResultUsm(ResultId resultId)
: resultId_(resultId) {}

ResultUsm::~ResultUsm()
{
    if (allocated_)
    {
        LOG_ERROR("Memory leak in ResultUsm with resultId " + std::to_string(resultId_) + ". Memory was not deallocated on device before destruction.");
    }
}

bool ResultUsm::allocateOnDevice(sycl::queue& queue)
{
    if (allocated_)
    {
        LOG_ERROR("Memory already allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return false;
    }

    try
    {
        deviceNumSolutions_ = sycl::malloc_device<u_int32_t>(1, queue);
        deviceSomeSolutionParameters_ = sycl::malloc_device<float>(MaxSolutions, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to allocate memory on device for ResultUsm with resultId " + std::to_string(resultId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = true;
    allocationQueue_ = &queue;
    return true;
}

bool ResultUsm::deallocateOnDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return false;
    }

    try
    {
        sycl::free(deviceNumSolutions_, queue);
        sycl::free(deviceSomeSolutionParameters_, queue);
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to deallocate memory on device for ResultUsm with resultId " + std::to_string(resultId_) + ". Exception: " + exception.what() + ".");
        return false;
    }

    allocated_ = false;
    allocationQueue_ = nullptr;
    return true;
}

DataUsm::TransferEvents ResultUsm::transferToDevice(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceNumSolutions_, &hostNumSolutions_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceSomeSolutionParameters_, hostSomeSolutionParameters_, hostNumSolutions_ * sizeof(float))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer data to device for ResultUsm with resultId " + std::to_string(resultId_) + ". Exception: " + exception.what() + ".");
        return TransferEvents{};
    }

    return transferEvents;
}

DataUsm::TransferEvents ResultUsm::transferToHost(sycl::queue& queue)
{
    if (!allocated_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(&hostNumSolutions_, deviceNumSolutions_, sizeof(u_int32_t))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSomeSolutionParameters_, deviceSomeSolutionParameters_, hostNumSolutions_ * sizeof(float))));
    }
    catch (sycl::exception& exception)
    {
        LOG_ERROR("Failed to transfer data to host for ResultUsm with resultId " + std::to_string(resultId_) + ". Exception: " + exception.what() + ".");
        return TransferEvents{};
    }

    return transferEvents;
}

bool ResultUsm::takeResourceGroup(const DeviceResourceGroup& resourceGroup, const sycl::queue& queue)
{
    if (resourceGroup.size() != 2)
    {
        LOG_ERROR("Invalid resource group size for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return false;
    }

    deviceNumSolutions_ = static_cast<u_int32_t*>(resourceGroup.at(DeviceResourceType::NumSolutions));
    deviceSomeSolutionParameters_ = static_cast<float*>(resourceGroup.at(DeviceResourceType::SomeSolutionParameters));

    resourcesBorrowed_ = true;
    allocationQueue_ = &queue;

    return true;
}

std::pair<std::unique_ptr<DeviceResourceGroup>, const sycl::queue*> ResultUsm::releaseResourceGroup()
{
    if (!resourcesBorrowed_)
    {
        LOG_ERROR("Resources not borrowed for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return {nullptr, nullptr};
    }

    auto resourceGroup = std::make_unique<DeviceResourceGroup>();
    resourceGroup->emplace(DeviceResourceType::NumSolutions, deviceNumSolutions_);
    resourceGroup->emplace(DeviceResourceType::SomeSolutionParameters, deviceSomeSolutionParameters_);

    resourcesBorrowed_ = false;
    allocationQueue_ = nullptr;

    return {std::move(resourceGroup), allocationQueue_};
}

std::unique_ptr<DeviceResourceGroup> ResultUsm::allocateDeviceResources(sycl::queue& queue)
{
    std::unique_ptr<DeviceResourceGroup> resourceGroup = std::make_unique<DeviceResourceGroup>();
    resourceGroup->emplace(DeviceResourceType::NumSolutions, sycl::malloc_device<u_int32_t>(1, queue));
    resourceGroup->emplace(DeviceResourceType::SomeSolutionParameters, sycl::malloc_device<float>(MaxSolutions, queue));

    return resourceGroup;
}

void ResultUsm::deallocateDeviceResources(DeviceResourceGroup& resourceGroup, sycl::queue& queue)
{
    sycl::free(resourceGroup.at(DeviceResourceType::NumSolutions), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::SomeSolutionParameters), queue);
}

