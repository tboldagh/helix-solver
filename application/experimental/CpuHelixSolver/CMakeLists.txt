helix_solver_add_library(CpuHelixSolver
TYPE
    STATIC

INCLUDE
    include

SRC
    src/Event.cpp
    src/Result.cpp
    src/Task.cpp
    src/HelixSolver.cpp

PUBLIC
    Logger
    SplitterUsm
)

helix_solver_add_library(CpuHelixSolverSuite
UNIT_TEST

LOCATION
    application/experimental/CpuHelixSolver

SRC
    test/HelixSolverSuite.cpp

PRIVATE
    CpuHelixSolver
    Logger
    RootEventLoader
)