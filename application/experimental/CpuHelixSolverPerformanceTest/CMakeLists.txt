helix_solver_add_library(CpuHelixSolverPerformanceTest
APPLICATION

INCLUDE
    include

SRC
    src/CpuHelixSolverPerformanceTest.cpp

PRIVATE
    Logger
    RootEventLoader
    CpuHelixSolver
)