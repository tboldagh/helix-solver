helix_solver_add_library(HelixSolverUsmDemo
APPLICATION
SYCL

INCLUDE
    include

SRC
    src/HelixSolverUsmDemo.cpp

PRIVATE
    HelixSolverUsm
    Logger
    RootEventLoader
    EventUsm
    SplitterUsm
)