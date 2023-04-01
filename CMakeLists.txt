cmake_minimum_required(VERSION 3.14)
project(helix-solver)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  nlohmann
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.2
  )

FetchContent_MakeAvailable(nlohmann)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS Core Hist Tree)

include_directories(include ${ROOT_INCLUDE_DIRS})

SET(libs nlohmann_json::nlohmann_json ${ROOT_LIBRARIES})


FILE(GLOB Sources src/*.cpp)

add_executable( ht_no_sycl ${Sources})

target_link_libraries(ht_no_sycl
    PRIVATE 
    ${libs}
)

add_executable( ht ${Sources})
target_link_libraries(ht
    PRIVATE 
    ${libs}
)
set_target_properties(ht 
    PROPERTIES 
    EXCLUDE_FROM_ALL true)
target_compile_definitions(ht PRIVATE USE_SYCL)

install(TARGETS ht_no_sycl)

add_subdirectory(test)



