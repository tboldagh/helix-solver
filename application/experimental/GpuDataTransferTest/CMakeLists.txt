helix_solver_add_library(GpuDataTransferTest
APPLICATION
SYCL

INCLUDE
    include

SRC
    src/GpuDataTransferTest.cpp
    src/TransferHeavyKernel.cpp

PRIVATE
    Debug
)