#include "GpuDataTransferTest/TransferHeavyKernel.h"

void TransferHeavyKernel::operator()(sycl::id<1> idx) const
{
    output[idx] = input[idx];
}