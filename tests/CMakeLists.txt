#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************
cmake_minimum_required (VERSION 3.0)

project (lwm2munittests)

set(LWM2MCORE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
include(${LWM2MCORE_ROOT_DIR}/lwm2mcore.cmake)

set(LWM2MCORE_TEST_SOURCES ${LWM2MCORE_SOURCES_DIR}/tests/tests.c)

add_definitions(${SHARED_DEFINITIONS} ${WAKAAMA_DEFINITIONS} ${LWM2MCORE_DEFINITIONS})

# Enable all warnings for this test build
add_definitions(-g
                -Wall
                -Wextra
                -Wfloat-equal
                -Wshadow
                -Wpointer-arith
                -Wcast-align
                -Wwrite-strings
                -Waggregate-return
                -Wswitch-default)
                #-Werror)

SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")

add_custom_target(
    coverage_report
    COMMAND lcov --capture --output-file coverage.info -d .
    COMMAND mkdir -p coverage_out
    COMMAND genhtml -o coverage_out coverage.info
    COMMAND echo
    COMMAND echo \"Report in coverage_out/\"
    COMMENT "Coverage report will be available in coverage_out/"
)

include_directories (${LWM2MCORE_SOURCES_DIR} ${WAKAAMA_SOURCES_DIR} ${TINYDTLS_SOURCES_DIR} ${LWM2MCORE_TEST_SOURCES})

add_custom_command(OUTPUT ${TINYDTLS_SOURCES_GENERATED} COMMAND ""  DEPENDS external_tinydtls)

file(GLOB SOURCES "*.c")

add_executable(${PROJECT_NAME} ${SOURCES} ${WAKAAMA_SOURCES} ${SHARED_SOURCES} ${LWM2MCORE_SOURCES} ${LWM2MCORE_TEST_SOURCES})

# Enable CMake Test Framework (CTest) which make testing available by
# a "test" target. For "make" this is "make test"
enable_testing()

set(APP_TARGET lwm2mcoreUnitTest)
target_link_libraries(lwm2munittests
                      cunit
                      -lssl
                      -lcrypto
                      -lz
                      -lgcov)

add_test(${APP_TARGET} ${EXECUTABLE_OUTPUT_PATH}/${APP_TARGET})
