helix_solver_add_library(GpuDataTransferTest
APPLICATION
SYCL

TYPE
    OBJECT

SRC
    src/GpuDataTransferTest.cpp

PRIVATE
    Debug
)