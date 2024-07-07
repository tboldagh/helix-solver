helix_solver_add_library(CernRoot
TYPE
    STATIC

INCLUDE
    ${ROOT_INCLUDE_DIRS}

SRC
    src/CernRoot.cpp

PUBLIC
    ${ROOT_LIBRARIES}
)