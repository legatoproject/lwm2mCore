/**
 * @file main.c
 *
 * LWM2MCore Linux client
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include "ctype.h"

#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/device.h>
#include <lwm2mcore/udp.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include <lwm2mcore/update.h>
#include "dtls_debug.h"
#include "dtlsConnection.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include <errno.h>
#include <signal.h>
#include "clientConfig.h"
#include "update.h"

//--------------------------------------------------------------------------------------------------
/**
 * CoAP max buffer size
 */
//--------------------------------------------------------------------------------------------------
#define MAX_PACKET_SIZE 1024

//--------------------------------------------------------------------------------------------------
/**
 * Socket configuration set in udp.c
 */
//--------------------------------------------------------------------------------------------------
extern lwm2mcore_SocketConfig_t LinuxSocketConfig;

//--------------------------------------------------------------------------------------------------
/**
 * File descriptor to be monitored
 */
//--------------------------------------------------------------------------------------------------
static fd_set Fd;

//--------------------------------------------------------------------------------------------------
/**
 * Static value for LWM2MCore context storage
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Ref_t ContextPtr = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Static value to know if the client needs to quit
 */
//--------------------------------------------------------------------------------------------------
static int Quit = 0;

//--------------------------------------------------------------------------------------------------
/**
 * LWM2MCore client endpoint
 */
//--------------------------------------------------------------------------------------------------
static char Endpoint[LWM2MCORE_ENDPOINT_LEN] = {0};

//--------------------------------------------------------------------------------------------------
/**
 * Static value for DTLS log level
 */
//--------------------------------------------------------------------------------------------------
static log_t LogLevel = DTLS_LOG_INFO;

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration
 */
//--------------------------------------------------------------------------------------------------
clientConfig_t* ClientConfiguration = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Supported Commands enumeration
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    START_CNX,          ///< Start a connection
    STOP_CNX,           ///< Stop a connection
    UPDATE_REQUEST,     ///< Send a registration update
    QUIT,               ///< Quit
    MAX_CMD             ///< Internal usage
}
AvcCommand_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for supported commands
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char*         namePtr;        ///< Command name
    const char*         shortDescPtr;   ///< Command short description
    AvcCommand_t        cmdId;          ///< Command Id
    void*               userDataPtr;    ///< User data
}
CommandDesc_t;

//--------------------------------------------------------------------------------------------------
/**
 * Function to stop the connection
 */
//--------------------------------------------------------------------------------------------------
static void StopConnection
(
    void
)
{
#ifdef LWM2M_DEREGISTER
    lwm2mcore_DisconnectWithDeregister(ContextPtr);
#else
    lwm2mcore_Disconnect(ContextPtr);
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Handler for LWM2MCore events
 */
//--------------------------------------------------------------------------------------------------
static int StatusHandler
(
    lwm2mcore_Status_t eventStatus              ///< [IN] event status
)
{
    switch (eventStatus.event)
    {
        case LWM2MCORE_EVENT_INITIALIZED:
            printf("LwM2MCore is initialized and ready to be used\n");
            break;

        case LWM2MCORE_EVENT_AGREEMENT_CONNECTION:
            printf("The device requests a user agreement to make a connection to the server\n");
            break;

        case LWM2MCORE_EVENT_AGREEMENT_DOWNLOAD:
            printf("The device requests a user agreement to make a connection to the server\n");
            break;

        case LWM2MCORE_EVENT_AGREEMENT_UPDATE:
            printf("The device requests a user agreement to install a downloaded package\n");
            break;

        case LWM2MCORE_EVENT_AUTHENTICATION_STARTED:
            printf("The OTA update client has started authentication with the server\n");
            break;

        case LWM2MCORE_EVENT_AUTHENTICATION_FAILED:
            printf("The OTA update client failed to authenticate with the server\n");
            break;

        case LWM2MCORE_EVENT_SESSION_STARTED:
            printf("The OTA update client succeeded in authenticating with the server and has "\
                   "started the session\n");
            break;

        case LWM2MCORE_EVENT_SESSION_FAILED:
            printf("The session with the server failed\n");
            break;

        case LWM2MCORE_EVENT_SESSION_FINISHED:
            printf("The session with the server finished successfully\n");
            lwm2mcore_Free(ContextPtr);
            ContextPtr = NULL;
            FD_CLR(LinuxSocketConfig.sock, &Fd);
            LinuxSocketConfig.sock = 0;
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS:
            printf("A descriptor was downloaded with the package size\n");

            ClientStartDownload(eventStatus.u.pkgStatus.pkgType,
                                eventStatus.u.pkgStatus.numBytes,
                                false);
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED:
            printf("The OTA update package downloaded successfully\n");
            lwm2mcore_Update(ContextPtr);
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED:
            printf("The OTA update package downloaded successfully, but could not be stored "\
                   "in flash\n");
            break;

        case LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_OK:
            printf("The OTA update package was certified to have been sent by a trusted "\
                   "server\n");
            break;

        case LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_NOT_OK:
            printf("The OTA update package was not certified to have been sent by a trusted "\
                   "server\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_STARTED:
            printf("An update package is being applied\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_FAILED:
            printf("The update failed\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_FINISHED:
            printf("The update succeeded\n");
            break;

        case LWM2MCORE_EVENT_FALLBACK_STARTED:
            printf("A fallback mechanism was started\n");
            break;

        case LWM2MCORE_EVENT_DOWNLOAD_PROGRESS:
            printf("Download progress %d\%%\n", eventStatus.u.pkgStatus.progress);
            break;

        case LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START:
            if (LWM2MCORE_SESSION_BOOTSTRAP == eventStatus.u.session.type)
            {
                printf("Connected to the Bootstrap server \n");
            }
            else
            {
                printf("Connected to the Device Management server \n");
            }
            break;

        case LWM2MCORE_EVENT_LWM2M_SESSION_INACTIVE:
            printf("Inactive session event\n");
            break;

        default:
            printf("Unknown event %d\n", eventStatus.event);
            break;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to catch CTRL-C
 */
//--------------------------------------------------------------------------------------------------
static void Interrupt
(
    int sigNumber      ///< [IN] Signal number
)
{
    char buffer[] ="...Please wait for program to exit...\n";
    (void)sigNumber;
    write(STDERR_FILENO, buffer, strlen(buffer));
    Quit = 1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to display help
 */
//--------------------------------------------------------------------------------------------------
static void PrintUsage
(
    void
)
{
    printf("Usage: LWM2MCore client [OPTION]\r\n");
    printf("Launch a LWM2M client.\r\n");
    printf("Options:\r\n");
    printf("  -d\t\tSet DTLS debug logs\r\n");
    printf("\r\n");
}

//--------------------------------------------------------------------------------------------------
/**
 * This is an array of commands described as { name, description, long description, callback,
 * userdata }.
 */
//--------------------------------------------------------------------------------------------------
CommandDesc_t Commands[] =
{
    {"start",   "Launch a connection to the server",    START_CNX,      NULL},
    {"stop",    "Stop a connection to the server",      STOP_CNX,       NULL},
    {"update",  "Trigger a registration update",        UPDATE_REQUEST, NULL},
    {"quit",    "Quit the client gracefully.",          QUIT,           NULL},
    {"^C",      "Quit the client abruptly.",            MAX_CMD,        NULL},
    {NULL,      NULL,                                   MAX_CMD,        NULL}
};

//--------------------------------------------------------------------------------------------------
/**
 * Function to find a command
 *
 * @return
 *      - Pointer to a command descriptor
 */
//--------------------------------------------------------------------------------------------------
static CommandDesc_t* FindCommand
(
    CommandDesc_t*  commandArrayPtr,    ///< [IN] Commands table
    uint8_t*        bufferPtr,          ///< [IN] Incoming command
    size_t          length              ///< [IN] Incoming command length
)
{
    int i = 0;

    if ( (NULL == commandArrayPtr)
     ||  (NULL == bufferPtr)
     || !length)
    {
        return NULL;
    }

    while ((NULL != commandArrayPtr[i].namePtr)
        && ((strlen(commandArrayPtr[i].namePtr) != length)
         || (strncmp((char*)bufferPtr, commandArrayPtr[i].namePtr, length))))
    {
        i++;
    }

    if (NULL == commandArrayPtr[i].namePtr)
    {
        return NULL;
    }

    return &commandArrayPtr[i];
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to display the help
 */
//--------------------------------------------------------------------------------------------------
static void DisplayHelp
(
    CommandDesc_t*  commandArrayPtr,    ///< [IN] Supported commands table
    uint8_t*        bufferPtr           ///< [IN] Incoming command
)
{
    printf("Command\tDescription\n");
    printf("-----------------------------------------\n");
    if ((NULL != commandArrayPtr) && (NULL != bufferPtr))
    {
        CommandDesc_t* cmdPtr;
        int length;

        // Find end of first argument
        length = 0;
        while ((bufferPtr[length] != 0) && (!isspace(bufferPtr[length]&0xff)))
        {
            length++;
        }

        cmdPtr = FindCommand(commandArrayPtr, bufferPtr, length);
        if (cmdPtr == NULL)
        {
            int i;
            for (i = 0 ; commandArrayPtr[i].namePtr != NULL ; i++)
            {
                printf("%s\t%s\n",
                        commandArrayPtr[i].namePtr,
                        commandArrayPtr[i].shortDescPtr);
            }
        }
        else
        {
            printf("%s\n", cmdPtr->shortDescPtr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to treat an incoming command
 */
//--------------------------------------------------------------------------------------------------
static void TreatCmd
(
    AvcCommand_t cmdId  ///< [IN] Command Id
)
{
    switch (cmdId)
    {
        case START_CNX:
            ContextPtr = lwm2mcore_Init(StatusHandler);
            if (NULL != ContextPtr)
            {
                // Register to the LWM2M agent
                size_t len = LWM2MCORE_ENDPOINT_LEN;
                if (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetDeviceImei(Endpoint, &len))
                {
                    uint16_t objNumber = lwm2mcore_ObjectRegister(ContextPtr, Endpoint, NULL, NULL);
                    if (!objNumber)
                    {
                        printf("ERROR in LWM2M object registration\n");
                    }
                    else
                    {
                        bool result = lwm2mcore_Connect(ContextPtr);
                        if (result != true)
                        {
                            printf("Connect error\n");
                        }
                    }
                }
                else
                {
                    printf("Error getting device IMEI/endpoint\n");
                }
            }
            break;

        case STOP_CNX:
            StopConnection();
        break;

        case UPDATE_REQUEST:
            lwm2mcore_Update(ContextPtr);
        break;

        case QUIT:
            if (NULL != ContextPtr)
            {
                lwm2mcore_DisconnectWithDeregister(ContextPtr);
                lwm2mcore_Free(ContextPtr);
            }
            Quit = 1;
        break;

        default:
            printf("Invalid command\n");
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to handle a command
 *
 */
//--------------------------------------------------------------------------------------------------
static void HandleCommand
(
    CommandDesc_t*  commandArrayPtr,    ///< Commands table
    uint8_t*        bufferPtr           ///< Incoming command
)
{
    CommandDesc_t* cmdPtr;
    int length = 0;

    // Find end of command name
    while (bufferPtr[length] != 0 && !isspace(bufferPtr[length]))
    {
        length++;
    }

    cmdPtr = FindCommand(commandArrayPtr, bufferPtr, length);
    if (NULL != cmdPtr)
    {
        TreatCmd(cmdPtr->cmdId);
    }
    else
    {
        printf("Unkown command\n");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * LWM2MCore main
 *
 * @return
 *      - EXIT_FAILURE on failure
 *      - EXIT_SUCCESS on success
 */
//--------------------------------------------------------------------------------------------------
int main
(
    int argc,           ///<[IN] argument count
    char* argvPtr[]     ///<[IN] argument vector
)
{
    int opt;
    int result;
    uint8_t buffer[MAX_PACKET_SIZE];
    struct sigaction psa;

    printf("#              #     #  #####  #     #  #####\n");
    printf("#       #    # ##   ## #     # ##   ## #     #  ####  #####  ######\n");
    printf("#       #    # # # # #       # # # # # #       #    # #    # #\n");
    printf("#       #    # #  #  #  #####  #  #  # #       #    # #    # #####\n");
    printf("#       # ## # #     # #       #     # #       #    # #####  #\n");
    printf("#       ##  ## #     # #       #     # #     # #    # #   #  #\n");
    printf("####### #    # #     # ####### #     #  #####   ####  #    # ######\n");
    printf("Copyright (C) Sierra Wireless Inc.\n\n");

    opt = 1;
    while (opt < argc)
    {
        if (argvPtr[opt] == NULL
            || argvPtr[opt][0] != '-'
            || argvPtr[opt][2] != 0)
        {
            PrintUsage();
            return 0;
        }
        switch (argvPtr[opt][1])
        {
            case 'd':
                printf("Set DTLS debug log\n");
                LogLevel = DTLS_LOG_DEBUG;
                break;

            default:
                PrintUsage();
                return 0;
        }
        opt += 1;
    }

    DisplayHelp(Commands, buffer);
    printf("Connection will be automatically launched in 5 seconds\n");
    sleep(5);

    // Set DTLS log level
    dtls_set_log_level(LogLevel);

    // Get the client configuration from clientConfig.txt file
    memset(&ClientConfiguration, 0, sizeof(ClientConfiguration));
    clientConfigRead(&ClientConfiguration);

    // Install signal handler to catch CTRL+C to gracefully shutdown
    psa.sa_handler = Interrupt;
    sigaction(SIGTSTP, &psa, NULL);

    // Automatically launch a connection
    TreatCmd(START_CNX);

    while (0 == Quit)
    {
        struct timeval tv;

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        FD_ZERO(&Fd);
        FD_SET(LinuxSocketConfig.sock, &Fd);
        FD_SET(STDIN_FILENO, &Fd);

        // This part will set up an interruption until an event happen on the socket until
        // "tv" timed out (set with the previous function)
        result = select(FD_SETSIZE, &Fd, NULL, NULL, &tv);

        if (result < 0)
        {
            if (errno != EINTR)
            {
                printf("Error in select(): %d %s\n", errno, strerror(errno));
            }
        }
        else if (result > 0)
        {
            int numBytes;

            // If the event happened on the SDTIN
            if (FD_ISSET(STDIN_FILENO, &Fd))
            {
                numBytes = read(STDIN_FILENO, buffer, MAX_PACKET_SIZE - 1);

                if (numBytes > 1)
                {
                    buffer[numBytes] = 0;
                    // We call the corresponding callback of the typed command passing it the buffer
                    // for further arguments
                    HandleCommand(Commands, buffer);
                }

                if (0 == Quit)
                {
                    printf("\r\n> ");
                    fflush(stdout);
                }
                else
                {
                    ClientConfigFree();
                    printf("\r\n");
                }
            }
            // If an event happens on the socket
            else if (FD_ISSET(LinuxSocketConfig.sock, &Fd))
            {
                struct sockaddr_storage addr;
                socklen_t addrLen = sizeof(addr);

                // We retrieve the data received
                numBytes = recvfrom(LinuxSocketConfig.sock,
                                    buffer,
                                    MAX_PACKET_SIZE,
                                    0,
                                    (struct sockaddr *)&addr,
                                    &addrLen);

                if (0 > numBytes)
                {
                    printf("Error in recvfrom(): %d %s\r\n", errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                else if (0 < numBytes)
                {
                    char s[INET6_ADDRSTRLEN];
                    in_port_t port;

                    if (AF_INET == addr.ss_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == addr.ss_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin6_port;
                    }
                    fprintf(stderr, "%d bytes received from [%s]:%hu\n", numBytes, s, ntohs(port));
                    lwm2mcore_DataDump("Received data", buffer, numBytes);
                    lwm2mcore_UdpReceiveCb(buffer, numBytes, &addr, addrLen, LinuxSocketConfig);
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}
