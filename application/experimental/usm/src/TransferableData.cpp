#include "EventUsm/TransferableData.h"

void TransferableData::setKernelMemory(KernelMemory* kernelMemory)
{
    kernelMemorySet_ = kernelMemory != nullptr;

    setKernelMemoryInternal(kernelMemory);
}