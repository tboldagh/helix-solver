helix_solver_add_library(HelixSolverTaskPerformanceTest
APPLICATION
SYCL

INCLUDE
    include

SRC
    src/HelixSolverTaskPerformanceTest.cpp

PRIVATE
    Logger
    RootEventLoader
    HelixSolverUsm
)