helix_solver_add_library(EventUsm
SYCL

TYPE
    OBJECT

INCLUDE
    include

SRC
    src/EventUsm.cpp

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