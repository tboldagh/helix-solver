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



find_package( IntelSYCL )
if( NOT IntelSYCL_FOUND )
   message( STATUS "SYCL compilation is not available, not building SYCL targets" )

   add_executable( ht_no_sycl ${Sources})

   target_link_libraries(ht_no_sycl
       PRIVATE
       ${libs}
   )

   target_compile_definitions(ht_no_sycl
       PRIVATE
       PRINT_DEBUG # control here debug messages
       PRINT_CDEBUG # control here cdebug messages
       PRINT_VERBOSE # control here verbose messages
   )
   install(TARGETS ht_no_sycl
       DESTINATION bin
   )

else()
    add_executable( ht_sycl ${Sources})
    target_link_libraries(ht_sycl
        PRIVATE
        ${libs}
    )
    target_compile_definitions(ht_sycl PRIVATE
        USE_SYCL=1
        R__HAS_STD_STRING_VIEW
    )
    add_sycl_to_target(TARGET ht_sycl SOURCES ${Sources})
    install(TARGETS ht_sycl DESTINATION bin
)

endif()


add_executable( spliter experimental/spliter.cpp src/ZPhiPartitioning.cpp )
target_include_directories( spliter PRIVATE /usr/local/include/sycl/ helix-solver/include/)
target_link_libraries( spliter PRIVATE  sycl )
target_compile_definitions( spliter PRIVATE USE_SYCL=0)
target_compile_options(spliter PRIVATE -fsycl -fsycl-targets=nvptx64-nvidia-cuda)
target_link_options( spliter PRIVATE -fsycl -fsycl-targets=nvptx64-nvidia-cuda)
install( TARGETS spliter DESTINATION bin
)




FILE(GLOB TBoxScripts tools/TBoxPlot/*.C)
message(STATUS " utility scipts " ${TBoxScripts})
install(FILES ${TBoxScripts} tools/TBoxPlot/make_tbox_plots.sh
    DESTINATION scripts
)


add_subdirectory(test)
