helix_solver_add_library(GpuDataTransferTest
APPLICATION
SYCL

TYPE
    OBJECT

INCLUDE
    include

SRC
    src/GpuDataTransferTest.cpp
    src/TransferHeavyKernel.cpp

PRIVATE
    Debug
)