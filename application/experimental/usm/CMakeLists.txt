helix_solver_add_library(EventUsm
SYCL

TYPE
    STATIC

INCLUDE
    include

SRC
    src/EventUsm.cpp
    src/ResultUsm.cpp
    src/ITask.cpp
    src/QueueUsm.cpp
    src/TaskUsm.cpp
    src/WorkerUsm.cpp

PUBLIC
    Logger
)

helix_solver_add_library(EventUsmSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/usm

SRC
    test/EventUsmSuite.cpp

PRIVATE
    EventUsm
    ILoggerMock
)

helix_solver_add_library(IQueueMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/IQueueMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(ITaskMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/ITaskMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(IWorkerMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/IWorkerMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(IWorkerControllerMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/IWorkerControllerMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(ITaskStateObserverMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/ITaskStateObserverMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(TaskUsmSuite
UNIT_TEST

LOCATION
    application/experimental/usm

SRC
    test/TaskUsmSuite.cpp

PRIVATE
    EventUsm
    ILoggerMock
    IQueueMock
    ITaskStateObserverMock
    IWorkerControllerMock
)

helix_solver_add_library(WorkerUsmSuite
UNIT_TEST

LOCATION
    application/experimental/usm

SRC
    test/WorkerUsmSuite.cpp

PRIVATE
    EventUsm
    ILoggerMock
    IQueueMock
    ITaskMock
    IWorkerControllerMock
)

helix_solver_add_library(QueueUsmSuite
UNIT_TEST

LOCATION
    application/experimental/usm

SRC
    test/QueueUsmSuite.cpp

PRIVATE
    EventUsm
    ILoggerMock
    ITaskMock
)