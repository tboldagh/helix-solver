function(helix_solver_add_library target)
    cmake_parse_arguments(
        ARG # prefix of output variables
        "UNIT_TEST;SYCL;APPLICATION"  # bolean arguments
        "LOCATION;TYPE"  # single value arguments
        "INCLUDE;INTERFACE;PRIVATE;PUBLIC;SRC"  # list as a value arguments
        ${ARGN} # arguments to parse
    )

    set(TARGET ${target})

    set(SRC_FILES ${ARG_SRC})

    if(ARG_UNIT_TEST)
        if(NOT ARG_LOCATION)
            message(FATAL_ERROR "LOCATION not provided for UNIT_TEST ${TARGET}. Provide LOCATION as a relative path from project's root directory to the directory containing ${TARGET}.")
        endif()

        add_executable(${TARGET} ${SRC_FILES})
        target_link_libraries(${TARGET} PUBLIC gtest_main gtest)
        add_test(NAME ${TARGET} COMMAND ${TARGET})

        set(UNIT_TESTS_EXECUTABLES ${UNIT_TESTS_EXECUTABLES} "${ARG_LOCATION}/${TARGET}" CACHE INTERNAL "TESTS_SOURCE_FILES" FORCE)
    elseif(ARG_APPLICATION)
        add_executable(${TARGET} ${SRC_FILES})
    else()
        if(NOT ARG_TYPE)
            message(FATAL_ERROR "TYPE not provided for non-UNIT_TEST, non-APPLICATION ${TARGET}. Provide TYPE as one of MODULE/OBJECT/SHARED/STATIC.")
        endif()

        add_library(${TARGET} ${ARG_TYPE} ${SRC_FILES})
    endif()

    if(ARG_SYCL)
        add_sycl_to_target(TARGET ${TARGET} SOURCES ${SRC_FILES})
    endif()

    target_link_libraries(
        ${TARGET}
        INTERFACE ${ARG_INTERFACE}
        PRIVATE ${ARG_PRIVATE}
        PUBLIC ${ARG_PUBLIC}
    )

    target_include_directories(${TARGET} PUBLIC ${ARG_INCLUDE})
endfunction()