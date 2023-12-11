helix_solver_add_library(HelixSolver
APPLICATION
SYCL

TYPE
    OBJECT

INCLUDE
    include

SRC
    src/AdaptiveHoughGpuKernel.cpp
    src/Application.cpp
    src/ComputingManager.cpp
    src/ComputingWorker.cpp
    src/EventBuffer.cpp
    src/Event.cpp
    src/main.cpp
    src/ZPhiPartitioning.cpp

PRIVATE
    CernRoot
    Debug
)