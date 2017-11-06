#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************
cmake_minimum_required (VERSION 3.0)

project (lwm2mcoreclient C CXX)

set(LWM2MCORE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include(${LWM2MCORE_ROOT_DIR}/wakaama.cmake)
include(${LWM2MCORE_ROOT_DIR}/tinydtls.cmake)
include(${LWM2MCORE_ROOT_DIR}/lwm2mcore.cmake)

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
                -DLWM2M_LOCATION_FLOAT)

include_directories (${LWM2MCORE_SOURCES_DIR} ${WAKAAMA_SOURCES_DIR} ${TINYDTLS_SOURCES_DIR})

set(LINUX_CLIENT_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/connectivity.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/clientConfig.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/debug.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/device.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/location.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/mutex.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/paramStorage.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/platform.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/security.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/sem.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/time.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/timer.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/udp.c
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/update.c
    main.c)

add_executable(${PROJECT_NAME} ${LWM2MCORE_SOURCES} ${LINUX_CLIENT_SOURCES})
target_link_libraries(${PROJECT_NAME} wakaama)
target_link_libraries(${PROJECT_NAME} tinydtls)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} -lrt)
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})



