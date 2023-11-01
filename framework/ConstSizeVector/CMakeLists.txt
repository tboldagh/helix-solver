helix_solver_add_library(ConstSizeVector
SYCL

TYPE
    OBJECT

INCLUDE
    include

SRC
    src/ConstSizeVector.cpp
)

helix_solver_add_library(ConstSizeVectorSuite
UNIT_TEST
SYCL

LOCATION
    framework/ConstSizeVector

SRC
    test/ConstSizeVectorSuite.cpp

PRIVATE
    ConstSizeVector
    UtSyclHelpers
)