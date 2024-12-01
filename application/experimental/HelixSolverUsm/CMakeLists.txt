helix_solver_add_library(HelixSolverUsm
SYCL

TYPE
    STATIC

INCLUDE
    include

SRC
    src/HelixSolverTask.cpp
    src/SingleRegionKernel.cpp
    src/HelixSolverQueue.cpp

PUBLIC
    EventUsm
    SplitterUsm
)

helix_solver_add_library(HelixSolverUsmSuite
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