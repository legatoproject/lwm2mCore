# --------------------------------------------------------------------------------------------------
# Generic file to generate linux client and tests
#
# Copyright (C) Sierra Wireless Inc.
# --------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.8)

# Set LwM2MCore source directory
set(LWM2MCORE_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/)

# Provides LWM2MCORE_SOURCES_DIR and LWM2MCORE_SOURCES and LWM2MCORE_DEFINITIONS variables.


include_directories(${LWM2MCORE_SOURCES_DIR}/examples/linux/
                    ${LEGATO_ROOT}/apps/platformServices/airVantageConnector/avcDaemon
                    ${LWM2MCORE_SOURCES_DIR}/include/
                    ${LWM2MCORE_SOURCES_DIR}/include/platform-specific/linux/
                    ${LWM2MCORE_SOURCES_DIR}/objectManager/
                    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/
                    ${LWM2MCORE_SOURCES_DIR}/sessionManager/
                    ${LWM2MCORE_SOURCES_DIR}/3rdParty/tinydtls/
                    ${LWM2MCORE_SOURCES_DIR}/3rdParty/tinyhttp/
                    ${LWM2MCORE_SOURCES_DIR}/3rdParty/wakaama/core/
                    ${LWM2MCORE_SOURCES_DIR}/3rdParty/wakaama/core/er-coap-13/)

set(LWM2MCORE_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/objectManager/aclConfiguration.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/bootstrapConfiguration.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/handlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/lwm2mcoreCoapHandlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objects.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objectsTable.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/utils.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/lwm2mcorePackageDownloader.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/fileTransfer.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/update.c
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
                -Werror
                -std=gnu99
                -DLWM2M_CLIENT_MODE
                -DLWM2M_BOOTSTRAP
                -DLWM2M_LITTLE_ENDIAN
                -DWITH_LOGS
                -DLWM2M_WITH_LOGS
                -DWITH_TINYDTLS
                -DLWM2M_OLD_CONTENT_FORMAT_SUPPORT
                -DSIERRA
                -DLWM2M_OBJECT_9)

if ($ENV{LE_CONFIG_AVC_FEATURE_FILETRANSFER} MATCHES "y")
    add_definitions(-DLWM2M_OBJECT_33406)
endif()