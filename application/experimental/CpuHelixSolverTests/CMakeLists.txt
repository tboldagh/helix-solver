helix_solver_add_library(CpuHelixSolverTests
APPLICATION

INCLUDE
    include

SRC
    src/CpuHelixSolverTests.cpp

PRIVATE
    CpuHelixSolver
    DataTypes
    Logger
    Readers
    RootEventLoader
    SpacepointsGenerator
)