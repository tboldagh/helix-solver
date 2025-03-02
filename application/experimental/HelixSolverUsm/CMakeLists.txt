helix_solver_add_library(HelixSolverUsm
SYCL

TYPE
    STATIC

INCLUDE
    include

SRC
    src/HelixSolverTask.cpp
    src/SingleRegionKernel.cpp
    src/SingleRegionKernelMemory.cpp
    src/HelixSolverQueue.cpp
    src/SumRegionNumSolutionsKernel.cpp
    src/ConcentrateSolutionsKernel.cpp

PUBLIC
    EventUsm
    SplitterUsm
)

helix_solver_add_library(HelixSolverTaskSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/HelixSolverUsm

SRC
    test/HelixSolverTaskSuite.cpp

PRIVATE
    HelixSolverUsm
    ITaskStateObserverMock
    Logger
    SplitterTest
)

helix_solver_add_library(SingleRegionKernelSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/HelixSolverUsm

SRC
    test/SingleRegionKernelSuite.cpp

PRIVATE
    HelixSolverUsm
)