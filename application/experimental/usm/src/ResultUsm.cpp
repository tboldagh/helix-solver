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
        deviceNumSolutions_ = sycl::malloc_device<decltype(hostNumSolutions_)>(1, queue);
        deviceNumRegionSolutions_ = sycl::malloc_device<std::remove_reference<decltype(*hostNumRegionSolutions_)>::type>(MaxRegions, queue);
        deviceSolutionHitCounts_ = sycl::malloc_device<std::remove_reference<decltype(*hostSolutionHitCounts_)>::type>(MaxSolutions, queue);
        deviceSolutionRs_ = sycl::malloc_device<std::remove_reference<decltype(*hostSolutionRs_)>::type>(MaxSolutions, queue);
        deviceSolutionPhis_ = sycl::malloc_device<std::remove_reference<decltype(*hostSolutionPhis_)>::type>(MaxSolutions, queue);
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
    if (!allocated_ && !resourcesBorrowed_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return false;
    }

    try
    {
        sycl::free(deviceNumSolutions_, queue);
        sycl::free(deviceNumRegionSolutions_, queue);
        sycl::free(deviceSolutionHitCounts_, queue);
        sycl::free(deviceSolutionRs_, queue);
        sycl::free(deviceSolutionPhis_, queue);
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
    if (!allocated_ && !resourcesBorrowed_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceNumSolutions_, &hostNumSolutions_, sizeof(hostNumSolutions_))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceNumRegionSolutions_, hostNumRegionSolutions_, MaxRegions * sizeof(hostNumRegionSolutions_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceSolutionHitCounts_, hostSolutionHitCounts_, MaxSolutions * sizeof(hostSolutionHitCounts_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceSolutionRs_, hostSolutionRs_, MaxSolutions * sizeof(hostSolutionRs_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(deviceSolutionPhis_, hostSolutionPhis_, MaxSolutions * sizeof(hostSolutionPhis_[0]))));
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
    if (!allocated_ && !resourcesBorrowed_)
    {
        LOG_ERROR("Memory not allocated on device for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return TransferEvents{};
    }

    TransferEvents transferEvents;
    try
    {
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(&hostNumSolutions_, deviceNumSolutions_, sizeof(hostNumSolutions_))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostNumRegionSolutions_, deviceNumRegionSolutions_, MaxRegions * sizeof(hostNumRegionSolutions_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionHitCounts_, deviceSolutionHitCounts_, MaxSolutions * sizeof(hostSolutionHitCounts_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionRs_, deviceSolutionRs_, MaxSolutions * sizeof(hostSolutionRs_[0]))));
        transferEvents.insert(std::make_unique<sycl::event>(queue.memcpy(hostSolutionPhis_, deviceSolutionPhis_, MaxSolutions * sizeof(hostSolutionPhis_[0]))));
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
    if (resourceGroup.size() != 5)
    {
        LOG_ERROR("Invalid resource group size for ResultUsm with resultId " + std::to_string(resultId_) + ".");
        return false;
    }

    deviceNumSolutions_ = static_cast<decltype(hostNumSolutions_)*>(resourceGroup.at(DeviceResourceType::NumSolutions));
    deviceNumRegionSolutions_ = static_cast<std::remove_reference<decltype(*hostNumRegionSolutions_)>::type*>(resourceGroup.at(DeviceResourceType::RegionNumSolutions));
    deviceSolutionHitCounts_ = static_cast<std::remove_reference<decltype(*hostSolutionHitCounts_)>::type*>(resourceGroup.at(DeviceResourceType::SolutionHitCounts));
    deviceSolutionRs_ = static_cast<std::remove_reference<decltype(*hostSolutionRs_)>::type*>(resourceGroup.at(DeviceResourceType::Rs));
    deviceSolutionPhis_ = static_cast<std::remove_reference<decltype(*hostSolutionPhis_)>::type*>(resourceGroup.at(DeviceResourceType::Phis));

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
    resourceGroup->emplace(DeviceResourceType::RegionNumSolutions, deviceNumRegionSolutions_);
    resourceGroup->emplace(DeviceResourceType::SolutionHitCounts, deviceSolutionHitCounts_);
    resourceGroup->emplace(DeviceResourceType::Rs, deviceSolutionRs_);
    resourceGroup->emplace(DeviceResourceType::Phis, deviceSolutionPhis_);

    resourcesBorrowed_ = false;
    allocationQueue_ = nullptr;

    return {std::move(resourceGroup), allocationQueue_};
}

std::unique_ptr<DeviceResourceGroup> ResultUsm::allocateDeviceResources(sycl::queue& queue)
{
    std::unique_ptr<DeviceResourceGroup> resourceGroup = std::make_unique<DeviceResourceGroup>();
    resourceGroup->emplace(DeviceResourceType::NumSolutions, sycl::malloc_device<decltype(hostNumSolutions_)>(1, queue));
    resourceGroup->emplace(DeviceResourceType::RegionNumSolutions, sycl::malloc_device<std::remove_reference<decltype(*hostNumRegionSolutions_)>::type>(MaxRegions, queue));
    resourceGroup->emplace(DeviceResourceType::SolutionHitCounts, sycl::malloc_device<std::remove_reference<decltype(*hostSolutionHitCounts_)>::type>(MaxSolutions, queue));
    resourceGroup->emplace(DeviceResourceType::Rs, sycl::malloc_device<std::remove_reference<decltype(*hostSolutionRs_)>::type>(MaxSolutions, queue));
    resourceGroup->emplace(DeviceResourceType::Phis, sycl::malloc_device<std::remove_reference<decltype(*hostSolutionPhis_)>::type>(MaxSolutions, queue));

    return resourceGroup;
}

void ResultUsm::deallocateDeviceResources(DeviceResourceGroup& resourceGroup, sycl::queue& queue)
{
    sycl::free(resourceGroup.at(DeviceResourceType::NumSolutions), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::RegionNumSolutions), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::SolutionHitCounts), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Rs), queue);
    sycl::free(resourceGroup.at(DeviceResourceType::Phis), queue);
}

