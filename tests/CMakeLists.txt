#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************
cmake_minimum_required (VERSION 2.8)

project (lwm2munittests)

set(LWM2MCORE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include(${LWM2MCORE_ROOT_DIR}/lwm2mcore.cmake)
include(${LWM2MCORE_ROOT_DIR}/3rdParty/tinyhttp.cmake)

if(OPENSSL)
    if(MBEDTLS)
    message(FATAL_ERROR "Both OPENSSL or MBEDTLS flags can not set at the same time")
    endif()
add_definitions(-DOPENSSL)
elseif(MBEDTLS)
add_definitions(-DMBEDTLS)
else()
message(WARNING "At least one flag needs to be set: OPENSSL or MBEDTLS. Default: OPENSSL")
add_definitions(-DOPENSSL)
endif()

include_directories(${LWM2MCORE_ROOT_DIR}/tests/)

set(LWM2MCORE_TEST_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/tests/tests.c
    ${LWM2MCORE_SOURCES_DIR}/tests/wakaama_stub.c
    ${LWM2MCORE_SOURCES_DIR}/tests/tinydtls_stub.c
    ${LWM2MCORE_SOURCES_DIR}/tests/download_stub.c
    ${LWM2MCORE_SOURCES_DIR}/tests/download_test.c
    ${LWM2MCORE_SOURCES_DIR}/tests/sampleConfig.c)

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
                -Wswitch-default
                -Werror)

SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

add_custom_target(
    coverage_report_lwm2mcore
    COMMAND lcov --capture --output-file coverage.info -d .
    COMMAND lcov --remove coverage.info *stub* test*.c tests.c alert.h dtls.h --output-file coverage.info
    COMMAND mkdir -p coverage_out
    COMMAND genhtml -o coverage_out coverage.info
    COMMAND echo
    COMMAND echo \"Report in coverage_out/\"
    COMMENT "Coverage report will be available in coverage_out/"
)

include_directories (${LWM2MCORE_SOURCES_DIR} ${WAKAAMA_SOURCES_DIR} ${TINYDTLS_SOURCES_DIR} ${LWM2MCORE_SOURCES_DIR}/tests)

set(LINUX_CLIENT_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/downloader.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/clientConfig.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/comm.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/connectivity.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/credentials.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/debug.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/device.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/location.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/packageCheck.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/paramStorage.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/platform.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/server.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/sslUtilities.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/time.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/timer.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/udp.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/update.c)

# Enable CMake Test Framework (CTest)
enable_testing()

add_executable(${PROJECT_NAME} ${LWM2MCORE_SOURCES} ${LINUX_CLIENT_SOURCES} ${LWM2MCORE_TEST_SOURCES})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} tinyhttp)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} -lrt)

target_link_libraries(${PROJECT_NAME}
                      -lssl
                      -lcrypto
                      -lz
                      -lgcov
                      -lrt)

if(NOT "${DATA_OUTPUT_PATH}" STREQUAL "")
    file(COPY data/http_301_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/http_404_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/http_500_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/bad_CRC_GET_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/bad_CRC_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/bad_signature_GET_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/bad_signature_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/image.png DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/image.png_GET_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/image.png_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/valid_package.bin DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/valid_package.bin_GET_response.txt DESTINATION ${DATA_OUTPUT_PATH})
    file(COPY data/valid_package.bin_HEAD_response.txt DESTINATION ${DATA_OUTPUT_PATH})
endif()

# Compile lwm2munittests
add_custom_target(lwm2munittests_compile COMMAND make)

# This is a C test
add_test(lwm2munittests ${EXECUTABLE_OUTPUT_PATH}/lwm2munittests)
