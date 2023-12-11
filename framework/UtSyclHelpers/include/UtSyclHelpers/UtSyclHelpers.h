#pragma once

#include <vector>

namespace UtSyclHelpers
{
    template<typename Type, typename AccessorType>
    void copyVariableFromBufferToHost(AccessorType& from, Type& to);

    template<typename Type, typename AccessorType>
    void copyBufferToHostVector(AccessorType& from, std::vector<Type>& to);


    template<typename Type, typename AccessorType>
    void copyVariableFromBufferToHost(AccessorType& from, Type& to)
    {
        auto hostAccerssor = from.get_host_access();
        to = hostAccerssor[0];
    }

    template<typename Type, typename AccessorType>
    void copyBufferToHostVector(AccessorType& from, std::vector<Type>& to)
    {
        for (uint32_t i = 0; i < from.size(); ++i)
        {
            to.push_back(from[i]);
        }
    }
}   // namespace UtSyclHelpers