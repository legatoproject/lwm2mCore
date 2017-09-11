#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************
cmake_minimum_required (VERSION 3.0)

project (lwm2mcoreclient C CXX)

set(LWM2MCORE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include(${LWM2MCORE_ROOT_DIR}/lwm2mcore.cmake)

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

include_directories (${LWM2MCORE_SOURCES_DIR} ${WAKAAMA_SOURCES_DIR} ${TINYDTLS_SOURCES_DIR})


find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

SET(LINUX_CLIENT_SOURCES main.c)


add_executable(${PROJECT_NAME} ${LINUX_CLIENT_SOURCES} ${WAKAAMA_SOURCES} ${SHARED_SOURCES} ${LWM2MCORE_SOURCES} ${LWM2MCORE_TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})

