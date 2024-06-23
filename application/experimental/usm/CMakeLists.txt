helix_solver_add_library(EventUsm
SYCL

TYPE
    OBJECT

INCLUDE
    include

SRC
    src/EventUsm.cpp
    src/ResultUsm.cpp
    src/ITask.cpp
    src/QueueUsm.cpp
    src/TaskUsm.cpp
    src/WorkerUsm.cpp

PRIVATE
    Debug
    Logger
)

helix_solver_add_library(EventUsmSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/EventUsm/test

SRC
    test/EventUsmSuite.cpp

PRIVATE
    EventUsm
    ILoggerMock
    Logger
)

helix_solver_add_library(IQueueMock
TYPE
    OBJECT

INCLUDE
    test-include

SRC
    test-src/IQueueMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(ITaskMock
TYPE
    OBJECT

INCLUDE
    test-include

SRC
    test-src/ITaskMock.cpp
)

helix_solver_add_library(IWorkerMock
TYPE
    OBJECT

INCLUDE
    test-include

SRC
    test-src/IWorkerMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(IWorkerControllerMock
TYPE
    OBJECT

INCLUDE
    test-include

SRC
    test-src/IWorkerControllerMock.cpp

PRIVATE
    EventUsm
)

helix_solver_add_library(ITaskStateObserverMock
TYPE
    OBJECT

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
    Logger
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
    Logger
)