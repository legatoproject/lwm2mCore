# --------------------------------------------------------------------------------------------------
# Generic file to generate linux client and tests
#
# Copyright (C) Sierra Wireless Inc.
# --------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.8)

# Set LwM2MCore source directory
set(LWM2MCORE_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/)

# Set tinyDTLS source file
set(TINYHTTP_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/3rdParty/tinyhttp)

set(TINYHTTP_SOURCES
    ${TINYHTTP_SOURCES_DIR}/chunk.c
    ${TINYHTTP_SOURCES_DIR}/header.c
    ${TINYHTTP_SOURCES_DIR}/http.c)

add_library(tinyhttp ${TINYHTTP_SOURCES})
target_compile_options(tinyhttp
                        PRIVATE
                        -g
                        -w
                        -DLWM2M_CLIENT_MODE
                        -DLWM2M_BOOTSTRAP
                        -DLWM2M_LITTLE_ENDIAN
                        -DWITH_LOGS
                        -DLWM2M_WITH_LOGS
                        -DWITH_TINYDTLS
                        -DLWM2M_OLD_CONTENT_FORMAT_SUPPORT
                        -std=gnu99
                        -DSIERRA)
