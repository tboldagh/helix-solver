helix_solver_add_library(SplitterUsm
SYCL

TYPE
    STATIC

INCLUDE
    include

SRC
    src/SplitterSettings.cpp
    src/TestDataLoader.cpp
    src/Splitter.cpp

PUBLIC
    ConstSizeVector
    EventUsm
)

helix_solver_add_library(TestDataLoaderSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/SplitterUsm

SRC
    test/TestDataLoaderSuite.cpp

PRIVATE
    SplitterUsm
)

helix_solver_add_library(SplitterTest
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/SplitterTest.cpp

PRIVATE
    SplitterUsm
)

helix_solver_add_library(SplitterSettingsSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/SplitterUsm

SRC
    test/SplitterSettingsSuite.cpp

PRIVATE
    SplitterUsm
)

helix_solver_add_library(SplitterSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/SplitterUsm

SRC
    test/SplitterSuite.cpp

PRIVATE
    SplitterTest
    SplitterUsm
    ILoggerMock
)