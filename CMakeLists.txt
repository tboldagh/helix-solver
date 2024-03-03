cmake_minimum_required(VERSION 3.16)

set(IntelSYCL_DIR "/opt/intel/oneapi/compiler/2023.2.1/linux/IntelSYCL")
set(ICX_PATH "/opt/intel/oneapi/compiler/2023.2.1/linux/bin/icx")
set(ICPX_PATH "/opt/intel/oneapi/compiler/2023.2.1/linux/bin/icpx")

# Must be set before project() call
set(CMAKE_C_COMPILER ${ICX_PATH})
set(CMAKE_CXX_COMPILER ${ICPX_PATH})
set(CMAKE_CXX_COMPILER_ID "IntelLLVM")

project(helix-solver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fsycl -fsycl-targets=nvptx64-nvidia-cuda")

# Debug options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Rno-debug-disables-optimization")

# Disable SYCL
# set(DISABLE_SYCL 1)

find_package(GTest REQUIRED)

find_package(IntelSYCL REQUIRED)

find_package(ROOT REQUIRED)

include(tools/cmake/helix_solver_add_library.cmake)

# Clear chached value
set(UNIT_TESTS_EXECUTABLES "" CACHE INTERNAL "TESTS_SOURCE_FILES" FORCE)


add_subdirectory(application)
add_subdirectory(framework)

# Save list of unit tests into TestSuitesList.txt file
set(UNIT_TEST_SUITES_LIST_FILE "${CMAKE_BINARY_DIR}/TestSuitesList.txt")
file(WRITE ${UNIT_TEST_SUITES_LIST_FILE} "")
foreach(executable ${UNIT_TESTS_EXECUTABLES})
    file(APPEND ${UNIT_TEST_SUITES_LIST_FILE} "${executable}\n")
endforeach()