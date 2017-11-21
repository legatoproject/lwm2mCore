# --------------------------------------------------------------------------------------------------
# Generic file to generate linux client and tests
#
# Copyright (C) Sierra Wireless Inc.
# --------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.8)

# Set LwM2MCore source directory
set(LWM2MCORE_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/)

# Provides LWM2MCORE_SOURCES_DIR and LWM2MCORE_SOURCES and LWM2MCORE_DEFINITIONS variables.

set(LWM2MCORE_HEADERS
    ${LWM2MCORE_SOURCES_DIR}/include/coapHandlers.h
    ${LWM2MCORE_SOURCES_DIR}/include/connectivity.h
    ${LWM2MCORE_SOURCES_DIR}/include/device.h
    ${LWM2MCORE_SOURCES_DIR}/include/location.h
    ${LWM2MCORE_SOURCES_DIR}/include/lwm2mcore.h
    ${LWM2MCORE_SOURCES_DIR}/include/paramStorage.h
    ${LWM2MCORE_SOURCES_DIR}/include/security.h
    ${LWM2MCORE_SOURCES_DIR}/include/sem.h
    ${LWM2MCORE_SOURCES_DIR}/include/socket.h
    ${LWM2MCORE_SOURCES_DIR}/include/timer.h
    ${LWM2MCORE_SOURCES_DIR}/include/udp.h
    ${LWM2MCORE_SOURCES_DIR}/include/update.h)

include_directories(${LWM2MCORE_SOURCES_DIR}/examples/linux/
                    ${LWM2MCORE_SOURCES_DIR}/include/
                    ${LWM2MCORE_SOURCES_DIR}/include/platform-specific/linux/
                    ${LWM2MCORE_SOURCES_DIR}/objectManager/
                    ${LWM2MCORE_SOURCES_DIR}/packageDownload/
                    ${LWM2MCORE_SOURCES_DIR}/sessionManager/
                    ${LWM2MCORE_SOURCES_DIR}/tinydtls/
                    ${LWM2MCORE_SOURCES_DIR}/wakaama/core/
                    ${LWM2MCORE_SOURCES_DIR}/wakaama/core/er-coap-13/)

set(LWM2MCORE_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/objectManager/handlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/lwm2mcoreCoapHandlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objects.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objectsTable.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/utils.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/lwm2mcorePackageDownloader.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/workspace.c
    ${LWM2MCORE_SOURCES_DIR}/sessionManager/dtlsConnection.c
    ${LWM2MCORE_SOURCES_DIR}/sessionManager/lwm2mcoreSession.c)

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
                -std=gnu99
                -DLWM2M_CLIENT_MODE
                -DLWM2M_BOOTSTRAP
                -DLWM2M_LITTLE_ENDIAN
                -DWITH_LOGS
                -DLWM2M_WITH_LOGS
                -DWITH_TINYDTLS
                -DLWM2M_OLD_CONTENT_FORMAT_SUPPORT
                -DSIERRA)
