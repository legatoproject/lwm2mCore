#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************
cmake_minimum_required (VERSION 2.8)

project (lwm2munittests)

set(LWM2MCORE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include(${LWM2MCORE_ROOT_DIR}/lwm2mcore.cmake)

set(LWM2MCORE_TEST_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/tests/tests.c
    ${LWM2MCORE_SOURCES_DIR}/tests/wakaama_stub.c
    ${LWM2MCORE_SOURCES_DIR}/tests/tinydtls_stub.c)

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

remove_definitions(-Werror)

SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")

add_custom_target(
    coverage_report_lwm2mcore
    COMMAND lcov --capture --output-file coverage.info -d .
    COMMAND mkdir -p coverage_out
    COMMAND genhtml -o coverage_out coverage.info
    COMMAND echo
    COMMAND echo \"Report in coverage_out/\"
    COMMENT "Coverage report will be available in coverage_out/"
)

include_directories (${LWM2MCORE_SOURCES_DIR} ${WAKAAMA_SOURCES_DIR} ${TINYDTLS_SOURCES_DIR} ${LWM2MCORE_SOURCES_DIR}/tests)

set(LINUX_CLIENT_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/clientConfig.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/connectivity.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/debug.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/device.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/location.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/paramStorage.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/security.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/time.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/timer.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/udp.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/server.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/platform.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/comm.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/update.c)

# Enable CMake Test Framework (CTest)
enable_testing()

add_executable(${PROJECT_NAME} ${LWM2MCORE_SOURCES} ${LINUX_CLIENT_SOURCES} ${LWM2MCORE_TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}
                      -lssl
                      -lcrypto
                      -lz
                      -lgcov
                      -lrt)

# Compile lwm2munittests
add_custom_target(lwm2munittests_compile COMMAND make)

# This is a C test
add_test(lwm2munittests ${EXECUTABLE_OUTPUT_PATH}/lwm2munittests)
