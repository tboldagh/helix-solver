helix_solver_add_library(Readers
TYPE
    STATIC

INCLUDE
    include

SRC
    src/SpacepointsReader.cpp
    src/ParticleInitialReader.cpp
    
PRIVATE
    CernRoot
    DataTypes
    Logger
)