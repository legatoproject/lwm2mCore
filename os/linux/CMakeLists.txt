cmake_minimum_required (VERSION 2.8)

project (lwm2mclient C CXX)

add_definitions(-DLWM2M_CLIENT_MODE -DLWM2M_BOOTSTRAP -DLWM2M_WITH_LOGS -DSIERRA)

FIND_PACKAGE ( Threads REQUIRED )
set(LWM2MCORE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)


set(TINYDTLS_SOURCES_DIR ${LWM2MCORE_DIR}/tinydtls)
set(TINYDTLS_SOURCES_GENERATED
    ${TINYDTLS_SOURCES_DIR}/tinydtls.h)

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
)
ExternalProject_Add_Step(external_tinydtls configure2
    COMMAND "autoreconf" "-i"
    ALWAYS 1
    WORKING_DIRECTORY "${TINYDTLS_SOURCES_DIR}"
    DEPENDERS configure
    DEPENDEES download
)
endif()

add_custom_command(OUTPUT ${TINYDTLS_SOURCES_GENERATED} COMMAND ""  DEPENDS external_tinydtls)

include_directories (${LWM2MCORE_DIR}/inc)
include_directories (${LWM2MCORE_DIR}/os/)

include_directories (${LWM2MCORE_DIR}/wakaama/core)
include_directories (${LWM2MCORE_DIR}/wakaama/core/er-coap-13)
include_directories (${LWM2MCORE_DIR}/wakaama/core/er-coap-13)

include_directories (${LWM2MCORE_DIR}/objectManager)
include_directories (${LWM2MCORE_DIR}/sessionManager)
include_directories (${LWM2MCORE_DIR}/tinydtls)

add_definitions(-DLWM2M_CLIENT_MODE
                -DLWM2M_BOOTSTRAP
                -DLWM2M_LITTLE_ENDIAN
                -DWITH_TINYDTLS
                -DLWM2M_WITH_LOGS
                -DWITH_LOGS
                -w
                -pthread
                -std=gnu99
                -DWITH_SHA256)

SET(SOURCES
    ${LWM2MCORE_DIR}/os/linux/osDebug.c
    ${LWM2MCORE_DIR}/os/linux/osPlatform.c
    ${LWM2MCORE_DIR}/os/linux/osTime.c
    ${LWM2MCORE_DIR}/os/linux/osTimer.c
    ${LWM2MCORE_DIR}/os/linux/osUdp.c
    ${LWM2MCORE_DIR}/os/linux/main.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcoreHandlers.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcoreObjects.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcoreObjectsTable.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcorePortDeviceDefault.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcorePortSecurityDefault.c
    ${LWM2MCORE_DIR}/objectManager/lwm2mcorePortUpdateDefault.c
    ${LWM2MCORE_DIR}/sessionManager/dtlsconnection.c
    ${LWM2MCORE_DIR}/sessionManager/lwm2mcoreSession.c
    ${LWM2MCORE_DIR}/tinydtls/dtls.c
    ${LWM2MCORE_DIR}/tinydtls/crypto.c
    ${LWM2MCORE_DIR}/tinydtls/ccm.c
    ${LWM2MCORE_DIR}/tinydtls/hmac.c
    ${LWM2MCORE_DIR}/tinydtls/dtls_debug.c
    ${LWM2MCORE_DIR}/tinydtls/netq.c
    ${LWM2MCORE_DIR}/tinydtls/peer.c
    ${LWM2MCORE_DIR}/tinydtls/dtls_time.c
    ${LWM2MCORE_DIR}/tinydtls/session.c
    ${LWM2MCORE_DIR}/tinydtls/sha2/sha2.c
    ${LWM2MCORE_DIR}/tinydtls/aes/rijndael.c
    ${LWM2MCORE_DIR}/tinydtls/ecc/ecc.c
    ${LWM2MCORE_DIR}/wakaama/core/er-coap-13/er-coap-13.c
    ${LWM2MCORE_DIR}/wakaama/core/bootstrap.c
    ${LWM2MCORE_DIR}/wakaama/core/block1.c
    ${LWM2MCORE_DIR}/wakaama/core/data.c
    ${LWM2MCORE_DIR}/wakaama/core/discover.c
    ${LWM2MCORE_DIR}/wakaama/core/json.c
    ${LWM2MCORE_DIR}/wakaama/core/liblwm2m.c
    ${LWM2MCORE_DIR}/wakaama/core/list.c
    ${LWM2MCORE_DIR}/wakaama/core/management.c
    ${LWM2MCORE_DIR}/wakaama/core/objects.c
    ${LWM2MCORE_DIR}/wakaama/core/observe.c
    ${LWM2MCORE_DIR}/wakaama/core/packet.c
    ${LWM2MCORE_DIR}/wakaama/core/registration.c
    ${LWM2MCORE_DIR}/wakaama/core/tlv.c
    ${LWM2MCORE_DIR}/wakaama/core/transaction.c
    ${LWM2MCORE_DIR}/wakaama/core/uri.c
    ${LWM2MCORE_DIR}/wakaama/core/utils.c
)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_executable(${PROJECT_NAME} ${SOURCES})

TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )

