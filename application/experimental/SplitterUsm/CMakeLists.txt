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

helix_solver_add_library(SplitterSuite
UNIT_TEST
SYCL

LOCATION
    application/experimental/SplitterUsm

SRC
    test/SplitterSuite.cpp

PRIVATE
    SplitterUsm
)