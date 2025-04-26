helix_solver_add_library(RootEventLoader
TYPE
    STATIC

INCLUDE
    include

SRC
    src/RootEventLoader.cpp

PRIVATE
    EventUsm
    CernRoot
    Logger
)

helix_solver_add_library(RootEventLoaderSuite
UNIT_TEST

LOCATION
    application/RootEventLoader

SRC
    test/RootEventLoaderSuite.cpp

PRIVATE
    RootEventLoader
    Logger
    EventUsm
    CernRoot
)
