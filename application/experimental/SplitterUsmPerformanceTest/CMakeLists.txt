helix_solver_add_library(SplitterUsmPerformanceTest
APPLICATION
SYCL

INCLUDE
    include

SRC
src/SplitterOnlyKernel.cpp
src/SplitterUsmPerformanceTest.cpp
src/SplitterOnlyTask.cpp

PRIVATE
    Logger
    SplitterUsm
    HelixSolverUsm
)