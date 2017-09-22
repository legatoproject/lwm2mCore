# --------------------------------------------------------------------------------------------------
# Generic file to generate linux client and tests
#
# Copyright (C) Sierra Wireless Inc.
# --------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.0)

# Set LwM2MCore source directory
set(LWM2MCORE_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/)

# Set Wakaama source dir
set(WAKAAMA_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/wakaama/core)
set(EXT_SOURCES ${WAKAAMA_SOURCES_DIR}/er-coap-13/er-coap-13.c)
set(CORE_HEADERS
    ${WAKAAMA_SOURCES_DIR}/liblwm2m.h
    ${LWM2MCORE_SOURCES_DIR}/include/platform-specific/linux
    ${LWM2MCORE_SOURCES_DIR}/tinydtls
    )
set(WAKAAMA_SOURCES
    ${WAKAAMA_SOURCES_DIR}/liblwm2m.c
    ${WAKAAMA_SOURCES_DIR}/uri.c
    ${WAKAAMA_SOURCES_DIR}/utils.c
    ${WAKAAMA_SOURCES_DIR}/objects.c
    ${WAKAAMA_SOURCES_DIR}/tlv.c
    ${WAKAAMA_SOURCES_DIR}/data.c
    ${WAKAAMA_SOURCES_DIR}/list.c
    ${WAKAAMA_SOURCES_DIR}/packet.c
    ${WAKAAMA_SOURCES_DIR}/transaction.c
    ${WAKAAMA_SOURCES_DIR}/registration.c
    ${WAKAAMA_SOURCES_DIR}/bootstrap.c
    ${WAKAAMA_SOURCES_DIR}/management.c
    ${WAKAAMA_SOURCES_DIR}/observe.c
    ${WAKAAMA_SOURCES_DIR}/json.c
    ${WAKAAMA_SOURCES_DIR}/discover.c
    ${WAKAAMA_SOURCES_DIR}/block1.c
    ${EXT_SOURCES})


# Set tinyDTLS source file
set(TINYDTLS_SOURCES_DIR ${LWM2MCORE_ROOT_DIR}/tinydtls)
set(TINYDTLS_SOURCES
    ${TINYDTLS_SOURCES_DIR}/dtls.h
    ${TINYDTLS_SOURCES_DIR}/dtls.c
    ${TINYDTLS_SOURCES_DIR}/crypto.c
    ${TINYDTLS_SOURCES_DIR}/ccm.c
    ${TINYDTLS_SOURCES_DIR}/hmac.c
    ${TINYDTLS_SOURCES_DIR}/dtls_debug.c
    ${TINYDTLS_SOURCES_DIR}/netq.c
    ${TINYDTLS_SOURCES_DIR}/peer.c
    ${TINYDTLS_SOURCES_DIR}/dtls_time.c
    ${TINYDTLS_SOURCES_DIR}/session.c
    ${TINYDTLS_SOURCES_DIR}/sha2/sha2.c
    ${TINYDTLS_SOURCES_DIR}/aes/rijndael.c
    ${TINYDTLS_SOURCES_DIR}/sha2/sha2.c
    ${TINYDTLS_SOURCES_DIR}/ecc/ecc.c)

set(TINYDTLS_SOURCES_GENERATED ${TINYDTLS_SOURCES_DIR}/tinydtls.h)

# source files are only available after tinydtls submodule have been checked out.
# Create a target "submodule_update" for that purpose.
find_package(Git REQUIRED)
add_custom_command(OUTPUT ${TINYDTLS_SOURCES} COMMAND ${GIT_EXECUTABLE} submodule update)
add_custom_target(submodule_update SOURCES ${TINYDTLS_SOURCES})

# The tinydtls configure step will create some more source files (tinydtls.h etc).
# Use cmake "External Project" modul to call autoreconf and configure on tinydtls if necessary.
if (NOT EXISTS ${TINYDTLS_SOURCES_GENERATED})
    include(ExternalProject)
    ExternalProject_Add(external_tinydtls
        SOURCE_DIR  "${TINYDTLS_SOURCES_DIR}"
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        CONFIGURE_COMMAND "${TINYDTLS_SOURCES_DIR}/configure"
        BUILD_IN_SOURCE 1
        LOG_DOWNLOAD 1
        LOG_CONFIGURE 1
        # Make the submodule_update target a dependency.
        DEPENDS submodule_update
        )

    ExternalProject_Add_Step(external_tinydtls configure2
       COMMAND "autoreconf" "-i"
       ALWAYS 1
       WORKING_DIRECTORY "${TINYDTLS_SOURCES_DIR}"
       DEPENDERS configure
       DEPENDEES download
    )

    # Let cmake know that it needs to execute the external_tinydtls target to generate those files.
    add_custom_command(OUTPUT ${TINYDTLS_SOURCES_GENERATED} COMMAND ""  DEPENDS external_tinydtls)
endif()

set(TINYDTLS_SOURCES ${TINYDTLS_SOURCES} ${TINYDTLS_SOURCES_GENERATED})

# Compile definitions for tinydtls
set_source_files_properties(${TINYDTLS_SOURCES} PROPERTIES COMPILE_DEFINITIONS WITH_SHA256)

set(SHARED_SOURCES ${TINYDTLS_SOURCES})
set(SHARED_INCLUDE_DIRS ${TINYDTLS_SOURCES_DIR})
set(SHARED_DEFINITIONS -DWITH_TINYDTLS)

# Automatically determine endianess. This can be overwritten by setting LWM2M_LITTLE_ENDIAN
# accordingly in a cross compile toolchain file.
if(NOT DEFINED LWM2M_LITTLE_ENDIAN)
    include(TestBigEndian)
    TEST_BIG_ENDIAN(LWM2M_BIG_ENDIAN)
    if (LWM2M_BIG_ENDIAN)
         set(LWM2M_LITTLE_ENDIAN FALSE)
    else()
         set(LWM2M_LITTLE_ENDIAN TRUE)
    endif()
endif()
if (LWM2M_LITTLE_ENDIAN)
    set(WAKAAMA_DEFINITIONS ${WAKAAMA_DEFINITIONS} -DLWM2M_LITTLE_ENDIAN)
endif()

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
                    ${WAKAAMA_SOURCES_DIR}/er-coap-13/)

set(LWM2MCORE_SOURCES
    ${LWM2MCORE_SOURCES_DIR}/examples/linux/connectivity.c
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
    ${LWM2MCORE_SOURCES_DIR}/objectManager/handlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/lwm2mcoreCoapHandlers.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objects.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/objectsTable.c
    ${LWM2MCORE_SOURCES_DIR}/objectManager/utils.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/lwm2mcorePackageDownloader.c
    ${LWM2MCORE_SOURCES_DIR}/packageDownloader/workspace.c
    ${LWM2MCORE_SOURCES_DIR}/sessionManager/dtlsConnection.c
    ${LWM2MCORE_SOURCES_DIR}/sessionManager/lwm2mcoreSession.c)

add_definitions(-DLWM2M_CLIENT_MODE
                -DLWM2M_BOOTSTRAP
                -DLWM2M_LITTLE_ENDIAN
                -DWITH_LOGS
                -DLWM2M_WITH_LOGS
                -DWITH_TINYDTLS
                -DDTLS
                -DLWM2M_OLD_CONTENT_FORMAT_SUPPORT
                -DSIERRA)
