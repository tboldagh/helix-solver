#pragma once

#include <unordered_map>


enum class DeviceResourceType : u_int8_t
{
    NumPoints,
    Xs,
    Ys,
    Zs,
    Layers,
    NumSolutions,
    SomeSolutionParameters
};

using DeviceResourceGroup = std::unordered_map<DeviceResourceType, void*>;