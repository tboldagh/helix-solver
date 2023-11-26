helix_solver_add_library(CernRoot
TYPE
    OBJECT

INCLUDE
    ${ROOT_INCLUDE_DIRS}

SRC
    src/CernRoot.cpp

PUBLIC
    ${ROOT_LIBRARIES}
)