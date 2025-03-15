#pragma once

#include "EventUsm/KernelMemory.h"

#include <unordered_map>
#include <sycl/sycl.hpp>

enum class DeviceResourceType : u_int8_t
{
    NumPoints,
    Xs,
    Ys,
    Zs,
    Layers,
    NumSolutions,
    RegionNumSolutions,
    SolutionHitCounts,
    Rs,
    Phis,
    Splitter,
    EventKernelMemory,
    ResultKernelMemory,
    KernelMemory,
    SplitterSettingsKernelMemory
};

using DeviceResourceGroup = std::unordered_map<DeviceResourceType, void*>;
// using DeviceResourceGroup = std::unordered_map<DeviceResourceType, KernelMemory*>;