/**
 * @file clientConfig.c
 *
 * Configuration file management for the LwM2MCore Linux client
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>
#include "liblwm2m.h"
#include "clientConfig.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum size for client configuration file
 */
//--------------------------------------------------------------------------------------------------
#define  MAX_FILE_SIZE                      2048

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length for a parameter
 */
//--------------------------------------------------------------------------------------------------
#define STR_BUF_LEN                         256

//--------------------------------------------------------------------------------------------------
/**
 * Maximum # of characters each config line has
 */
//--------------------------------------------------------------------------------------------------
#define MAX_LINE                            128

//--------------------------------------------------------------------------------------------------
/**
 * Maximum # of characters of the section name
 */
//--------------------------------------------------------------------------------------------------
#define MAX_SECTION                         32

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration filename
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_FILENAME             "clientConfig.txt"

//--------------------------------------------------------------------------------------------------
/**
 * Configuration filename maximum length
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_FILENAME_MAX_LENGTH   20

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for configuration file: sections
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    CLIENT_CONFIG_GENERAL_SECTION,      ///< General section
    CLIENT_CONFIG_SECURITY_SECTION,     ///< Security section
    CLIENT_CONFIG_SERVER_SECTION,       ///< Server section
    CLIENT_CONFIG_SECTION_MAX           ///< Internal usage
}
ClientConfigSectionTypeEnum_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for a parameter defined in the configuration file
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char*                 namePtr;        ///< Parameter name
    lwm2mcore_ResourceType_t    dataType;       ///< Parameter data type
    size_t                      dataOffset;     ///< Offset of the field from the start of the
                                                ///< associated data structure
}
ParameterTable_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for a section defined in the configuration file
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    const char*                 sectionNamePtr;     ///< Section name
}
SectionTable_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for the general section of the configuration file
 */
//--------------------------------------------------------------------------------------------------
static ParameterTable_t GeneralConfig[] = {
    {
        CLIENT_CONFIG_ENDPOINT,
        LWM2MCORE_RESOURCE_TYPE_STRING,
        offsetof(clientGeneralConfig_t,IMEI)
    },
    {
        CLIENT_CONFIG_SERIAL_NUMBER,
        LWM2MCORE_RESOURCE_TYPE_STRING,
        offsetof(clientGeneralConfig_t, SN)
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Structure for the bootstrap and DM servers credentials of the configuration file
 */
//--------------------------------------------------------------------------------------------------
static ParameterTable_t SecurityConfig[] = {
    {
        CLIENT_CONFIG_SERVER_URL,
        LWM2MCORE_RESOURCE_TYPE_STRING,
        offsetof(clientSecurityConfig_t, serverURI)
    },
    {
        CLIENT_CONFIG_SERVER_PSKID,
        LWM2MCORE_RESOURCE_TYPE_STRING,
        offsetof(clientSecurityConfig_t, devicePKID)
    },
    {
        CLIENT_CONFIG_SERVER_PSK,
        LWM2MCORE_RESOURCE_TYPE_STRING,
        offsetof(clientSecurityConfig_t, secretKey)
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Section names of the configuration file
 */
//--------------------------------------------------------------------------------------------------
static SectionTable_t SectionTable[] =
{
    {CLIENT_CONFIG_GENERAL_SECTION_NAME     },
    {CLIENT_CONFIG_BS_SERVER_SECTION_NAME   },
    {CLIENT_CONFIG_DM_SERVER_SECTION_NAME   }
};

//--------------------------------------------------------------------------------------------------
/**
 * Global configuration data
 */
//--------------------------------------------------------------------------------------------------
static clientConfig_t   ClientConfig;

//--------------------------------------------------------------------------------------------------
/**
 * Handler prototype called when a parameter is found in the configuration file
 */
//--------------------------------------------------------------------------------------------------
typedef int (*HandlerFunc)
(
    uint8_t*            dataPtr,        ///< [IN] Client configuration pointer
    const char*         sectionPtr,     ///< [IN] Section the name of the section
    const char*         namePtr,        ///< [IN] Configuration name
    const char*         valuePtr,       ///< [IN] Configuration value
    lwm2mcore_OpType_t  operation       ///< [IN] Operation
);

//--------------------------------------------------------------------------------------------------
/**
 * Return pointer to last non space string
 *
 * @return
 *  - Updated string pointer on success
 */
//--------------------------------------------------------------------------------------------------
static char* SkipTrailingSpace
(
    const char* tailPtr,    ///< [IN] String end address
    const char* strPtr      ///< [IN] String to be treated
)
{
    assert(strPtr != NULL);
    assert(tailPtr != NULL);
    while ((tailPtr > strPtr) && (*(tailPtr - 1) <= ' '))
    {
        tailPtr--;
    }

    return (char*)tailPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Strip trailing space
 *
 * @return
 *  - Updated string pointer on success
 */
//--------------------------------------------------------------------------------------------------
static char* RemoveTrailingSpace
(
    const char* strPtr      ///< [IN] String to be treated
)
{
    assert(strPtr != NULL);
    char *pPtr = SkipTrailingSpace(strchr(strPtr, '\0'), strPtr);
    assert(pPtr != NULL);
    *pPtr = '\0';

    return (char *)strPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Find a character in a string
 *
 * @return
 *  - return pointer to first requested character
 *
 */
//--------------------------------------------------------------------------------------------------
static char* FindChar
(
    const char* strPtr,     ///< [IN] String to check
    const char  c           ///< [IN] Character to be found
)
{
    while ( (strPtr) && (*strPtr) && (*strPtr != c))
    {
        strPtr++;
    }

    return (char*)strPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Return pointer to first non space string
 *
 * @return
 *  - return pointer to first non space string
 *
 */
//--------------------------------------------------------------------------------------------------
static char* SkipLeadingSpace
(
    const char* strPtr      ///< [IN] String to analyze
)
{
    assert(strPtr != NULL);
    while ((*strPtr != '\0') && (*strPtr <= ' '))
    {
        strPtr++;
    }
    return (char *)strPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read or write configuration value
 *
 * For READ, the function converts the string "valuePtr" based on dataType, and assign it to "data".
 * For WRITE, the function cast the "data" based on dataType and convert it to string into
 * "valuePtr".
 *
 * NOTE: for WRITE, make sure the buffer "valuePtr" is large enough to hold result string.
 *
 * @return
 *  - 0 on success
 *  - -1 on failure.
 *
 */
//--------------------------------------------------------------------------------------------------
static int ReadWriteValue
(
    void*                       dataPtr,    ///< [IN] Client configuration pointer
    lwm2mcore_ResourceType_t    dataType,   ///< [IN] Data type
    char*                       valuePtr,   ///< [IN] Buffer contains the input/output string
    lwm2mcore_OpType_t          operation   ///< [IN] Operation
)
{
    if ((LWM2MCORE_OP_WRITE != operation) && (LWM2MCORE_OP_READ != operation))
    {
        return -1;
    }

    switch (dataType)
    {
        case LWM2MCORE_RESOURCE_TYPE_INT:
            if (LWM2MCORE_OP_READ == operation)
            {
                *(int*)dataPtr = atoi(valuePtr);
            }
            else
            {
                sprintf(valuePtr, "%d", *(int *)dataPtr);
            }
            break;

        case LWM2MCORE_RESOURCE_TYPE_BOOL:
            if (LWM2MCORE_OP_READ == operation)
            {
                *(bool*)dataPtr = strcasecmp(valuePtr, "TRUE") ? false : true;
            }
            else
            {
                sprintf(valuePtr, "%s", *((bool *)dataPtr) == true ? "TRUE" : "FALSE");
            }
            break;

        case LWM2MCORE_RESOURCE_TYPE_STRING:
            if (LWM2MCORE_OP_READ == operation)
            {
                strcpy((char*)dataPtr, valuePtr);
            }
            else
            {
                strcpy(valuePtr, (char*)dataPtr);
            }
            break;

        default:
            printf("Unhandled configuration data type %d\n", dataType);
            return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get security configuration for a specific server Id
 *
 * @return
 *  - Pointer on security object instance on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
clientSecurityConfig_t* GetDmServerConfigById
(
    uint16_t            serverId            ///< Server Id
)
{
    clientSecurityConfig_t* securityConfigurationPtr;

    securityConfigurationPtr = ClientConfig.securityPtr;
    while (securityConfigurationPtr)
    {
        if ((false == securityConfigurationPtr->isBootstrapServer)
         && (securityConfigurationPtr->serverId == serverId))
        {
            return securityConfigurationPtr;
        }
        securityConfigurationPtr = securityConfigurationPtr->nextPtr;
    }
    return securityConfigurationPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get security information for the bootstrap server
 *
 * @return
 *  - Pointer on security object instance on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
clientSecurityConfig_t* GetBootstrapInformation
(
    void
)
{
    clientSecurityConfig_t* securityConfigurationPtr;

    securityConfigurationPtr = ClientConfig.securityPtr;
    while ((securityConfigurationPtr) && (false == securityConfigurationPtr->isBootstrapServer))
    {
        securityConfigurationPtr = securityConfigurationPtr->nextPtr;
    }
    return securityConfigurationPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 0 (security) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
static void AddSecurity
(
    clientConfig_t*         clientConfigPtr,            ///< [IN] Client  configuration
    clientSecurityConfig_t* securityInformationPtr      ///< [IN] Security object
)
{
    if (!clientConfigPtr->securityPtr)
    {
        clientConfigPtr->securityPtr = securityInformationPtr;
        clientConfigPtr->securityPtr->nextPtr = NULL;
    }
    else
    {
        clientSecurityConfig_t* tempPtr = clientConfigPtr->securityPtr;

        while (tempPtr->nextPtr)
        {
            tempPtr = tempPtr->nextPtr;
        }
        tempPtr->nextPtr = securityInformationPtr;
        securityInformationPtr->nextPtr = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Callback function for configuration file parsing.
 *
 * The handler is called every time a a configuration name/value is found in configuration file.
 *
 * @return
 *  - 0 on success
 *  - -1 on failure
 */
//--------------------------------------------------------------------------------------------------
static int ConfigurationHandler
(
    uint8_t*            dataPtr,        ///< [IN] Client configuration pointer
    const char*         sectionPtr,     ///< [IN] Section the name of the section
    const char*         namePtr,        ///< [IN] Configuration name
    const char*         valuePtr,       ///< [IN] Configuration value
    lwm2mcore_OpType_t  operation       ///< [IN] Operation
)
{
    uint32_t sectionId = 0;
    uint32_t parameterId = 0;

    for (sectionId = 0; sectionId < (sizeof(SectionTable)/sizeof(SectionTable_t)); sectionId++)
    {
        if (!strncmp(SectionTable[sectionId].sectionNamePtr, sectionPtr, strlen(sectionPtr)))
        {
            // Section was found
            switch (sectionId)
            {
                case CLIENT_CONFIG_GENERAL_SECTION:
                    for (parameterId = 0;
                         parameterId < (sizeof(GeneralConfig)/sizeof(ParameterTable_t));
                         parameterId++)
                    {
                        if (!strncmp(GeneralConfig[parameterId].namePtr, namePtr, strlen(namePtr)))
                        {
                            // Parameter was found
                            return ReadWriteValue(dataPtr + GeneralConfig[parameterId].dataOffset,
                                                  GeneralConfig[parameterId].dataType,
                                                  (char*)valuePtr,
                                                  operation);
                        }
                    }
                    break;

                case CLIENT_CONFIG_SECURITY_SECTION:
                    for (parameterId = 0;
                         parameterId < (sizeof(SecurityConfig)/sizeof(ParameterTable_t));
                         parameterId++)
                    {
                        if (!strncmp(SecurityConfig[parameterId].namePtr, namePtr, strlen(namePtr)))
                        {
                            // Parameter was found

                            uint8_t* ptr;
                            clientSecurityConfig_t* securityConfigPtr = GetBootstrapInformation();
                            if (!securityConfigPtr)
                            {
                                securityConfigPtr = (clientSecurityConfig_t*)
                                                    lwm2m_malloc(sizeof(clientSecurityConfig_t));
                                memset(securityConfigPtr, 0, sizeof(clientSecurityConfig_t));
                                securityConfigPtr->isBootstrapServer = true;
                                AddSecurity(&ClientConfig, securityConfigPtr);
                            }

                            ptr = (uint8_t*)(securityConfigPtr);
                            return ReadWriteValue((uint8_t*)
                                                  (ptr + SecurityConfig[parameterId].dataOffset),
                                                  SecurityConfig[parameterId].dataType,
                                                  (char*)valuePtr,
                                                  operation);
                        }
                    }
                    break;

                case CLIENT_CONFIG_SERVER_SECTION:
                    for (parameterId = 0;
                         parameterId < (sizeof(SecurityConfig)/sizeof(ParameterTable_t));
                         parameterId++)
                    {
                        if (!strncmp(SecurityConfig[parameterId].namePtr,
                                     namePtr,
                                     strlen(SecurityConfig[parameterId].namePtr)))
                        {
                            // Parameter was found

                            // Get the server Id
                            clientSecurityConfig_t* securityConfigPtr;
                            char *aData;
                            char* cSavePtr = NULL;
                            uint8_t* ptr;
                            uint16_t serverId;
                            strtok_r((char*)namePtr, " ", &cSavePtr);
                            aData = strtok_r(NULL, " ", &cSavePtr);
                            serverId = atoi(aData);

                            securityConfigPtr = GetDmServerConfigById(serverId);
                            if (!securityConfigPtr)
                            {
                                securityConfigPtr = (clientSecurityConfig_t*)
                                                     lwm2m_malloc(sizeof(clientSecurityConfig_t));
                                memset(securityConfigPtr, 0, sizeof(clientSecurityConfig_t));
                                securityConfigPtr->isBootstrapServer = false;
                                securityConfigPtr->serverId = serverId;
                                AddSecurity(&ClientConfig, securityConfigPtr);
                            }
                            ptr = (uint8_t*)(securityConfigPtr);
                            return ReadWriteValue((uint8_t*)
                                                  (ptr + SecurityConfig[parameterId].dataOffset),
                                                  SecurityConfig[parameterId].dataType,
                                                  (char*)valuePtr,
                                                  operation);
                        }
                    }
                    break;

                default:
                    printf("Invalid section Id %d\n", sectionId);
                    break;
            }
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse the configuration file, call handler for every key/value pair.
 *
 * @return
 *  - 0 on success
 *  - -1 on failure.
 */
//--------------------------------------------------------------------------------------------------
static int ParseConfigFile
(
    FILE*               fPtr,       ///< [IN] Filename name of the configuration file
    HandlerFunc         handler,    ///< [IN] pointer to the handler function
    void*               dataPtr,    ///< [IN] Client configuration pointer
    lwm2mcore_OpType_t  operation   ///< [IN] Operation
)
{
    char line[MAX_LINE];
    char section[MAX_SECTION];
    char* firstPtr = 0;
    char* lastPtr = 0;
    char* namePtr = 0;
    char* valuePtr = 0;
    int err = 0;
    int index = 0;

    while (fgets(line, MAX_LINE, fPtr) != NULL)
    {
        index++;

        firstPtr = line;
        firstPtr = RemoveTrailingSpace(SkipLeadingSpace(firstPtr));

        if (*firstPtr == 0)
        {
            continue;
        }
        else if ((';' == *firstPtr) || ('#' == *firstPtr))
        {
            // Comment line
            continue;
        }
        else if ('[' == *firstPtr)
        {
            // Section name
            lastPtr = FindChar(firstPtr + 1, ']');

            if (']' == *lastPtr)
            {
                *lastPtr = '\0';
                assert(strlen(firstPtr + 1) < MAX_SECTION);
                strncpy(section, firstPtr + 1, MAX_SECTION);
            }
            else
            {
                err = index;
                break;
            }
        }
        else if (*firstPtr)
        {
            // Not comment, not section, must be name = value pair
            lastPtr = FindChar(firstPtr, '=');

            if ('=' == *lastPtr)
            {
                *lastPtr = '\0';
                namePtr = RemoveTrailingSpace(firstPtr);
                valuePtr = SkipLeadingSpace(lastPtr + 1);
                // Find out if it's "namevaluePtr = valuePtr ;comment"
                lastPtr = FindChar(valuePtr, ';');
                if (*lastPtr == ';')
                {
                    *lastPtr = '\0';
                    valuePtr = RemoveTrailingSpace(valuePtr);
                }
                if (handler(dataPtr, section, namePtr, valuePtr, operation))
                {
                    err = index;
                    break;
                }
            }
            else
            {
                err = index;
                break;
            }
        }
    }

    if (err)
    {
        printf("Configuration file parsing error at line %d\n", err);
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read the configuration file
 *
 * @return
 *  - Buffer size
 */
//--------------------------------------------------------------------------------------------------
static int ReadFileToBuffer
(
    char* bufferPtr     ///< [IN] Buffer in which configuration file content is copied
)
{
    FILE* fPtr = NULL;
    int bsize = MAX_FILE_SIZE;
    char fname[CLIENT_CONFIG_FILENAME_MAX_LENGTH];

    sprintf(fname, "%s", CLIENT_CONFIG_FILENAME);

    if (NULL != (fPtr = fopen(fname, "r")))
    {
        if ((bsize = fread(bufferPtr, 1, bsize, fPtr)) == 0)
        {
            printf("Failed to read file %s\n", fname);
            fclose(fPtr);
            return 0;
        }
        fclose(fPtr);
        return bsize;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write the configuration file
 *
 * @return
 *  - Written buffer size
 */
//--------------------------------------------------------------------------------------------------
static int WriteBufferToFile
(
    char*  bufferPtr,       ///< [IN] Data to be written in configuration file
    size_t bsize            ///< [IN] Data length
)
{
    FILE* fPtr = NULL;
    char fname[CLIENT_CONFIG_FILENAME_MAX_LENGTH];

    sprintf(fname, "%s", CLIENT_CONFIG_FILENAME);

    if (NULL != (fPtr = fopen(fname, "w")))
    {
        if (fwrite(bufferPtr, 1, bsize, fPtr) != bsize)
        {
            fclose(fPtr);
            printf("%s Failed to write file: %s\n", __func__, fname);
            return 0;
        }
        fclose(fPtr);
        return bsize;
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write a single configuration line (name/value pair) into bufferPtr.
 * The original configuration file content is stored in bufferPtr, the function does in-memory
 * manipulation.
 *
 *  1. if the section and name is found, the function updates the configuration value to nvaluePtr.
 *  2. if the section is found, but configuration name in that section is not found, it adds the
 *     name/value to the section.
 *  3. if both section and configuration name are not found, it adds the new section, and adds the
 *     name/value to the new section.
 *
 *  The function doesn't check if configuration file is valid, it's assumed sanity is checked during
 *  parsing of the file, which happens before writing/updating the file.
 *
 *  @return
 *  - size of the result buffer on success
 *  - -1 on failure.
 *
 */
//--------------------------------------------------------------------------------------------------
static int WriteConfigLine
(
    char*       bufferPtr,      ///< [IN] Buffer containing the configuration file
    int         bsize,          ///< [IN] Size of the buffer
    const char* nsectionPtr,    ///< [IN] Section name
    const char* nnamePtr,       ///< [IN] Configuration name
    const char* nvaluePtr       ///< [IN] Configuration value
)
{
    char line[MAX_LINE];    // Line buffer
    bool sMatch = false;    // Matched section
    bool nMatch = false;    // Matched name
    bool lMatch = false;    // Matched line (both section and name matched)
    int lineSize = 0;
    int newLineSize = 0;
    int osize = bsize;      // Save original buffer size
    char* tokenPtr = NULL;
    char* sectionPtr = NULL;
    char* namePtr = NULL;
    char* positionPtr;      // Position pointer

    // Scan through buffer, line by line
    while(sscanf(bufferPtr, "%127[^\n]", line) > 0 ||
            sscanf(bufferPtr, "%127[ \n\t]", line) > 0)
    {

        // Current processing line
        positionPtr = bufferPtr;

        if (strlen(line) == 1)
        {
            // Empty line that has LF only
            lineSize = 1;
        }
        else if (line[0] == '\n')
        {
            // Mulitple LF
            lineSize = strlen(line);
        }
        else
        {
            // String +  LF
            lineSize = strlen(line) + 1;
        }
        // Next line
        bufferPtr += lineSize;
        // Remaining buffer size
        bsize -= lineSize;

        tokenPtr = RemoveTrailingSpace(SkipLeadingSpace(line));
        // Reset name match flag
        nMatch = false;
        lMatch = false;
        if ((0 == *tokenPtr) || (';' == *tokenPtr) || ('#' == *tokenPtr))
        {
            // Comment or empty line
        }
        else if ('[' == *tokenPtr )
        {
            // Section start
            sectionPtr = tokenPtr + 1;
            tokenPtr = FindChar(tokenPtr + 1, ']');
            if (']' == *tokenPtr)
            {
                *tokenPtr = '\0';
                if (strcasecmp(sectionPtr, nsectionPtr))
                {
                    if ((sMatch == true) && (lMatch == false))
                    {
                        // We went through matching section, but didn't find matching
                        // config name, add the name/value to the end of matching section,
                        // before new section.
                        bzero(line, MAX_LINE);
                        newLineSize = snprintf(line, MAX_LINE, "%s = %s\n", nnamePtr, nvaluePtr);
                        // Assume buffer is big enough
                        memmove(positionPtr + newLineSize, positionPtr, bsize + lineSize );
                        memcpy(positionPtr, line, newLineSize);
                        // We are done!
                        return osize + newLineSize;

                    }
                    sMatch = false;
                }
                else
                {
                    sMatch = true;
                }
            }
            else
            {
                // Syntax error
                printf("syntax error: expect ']' %s\n", line);
                return -1;
            }
        }
        else if (*tokenPtr)
        {
            // Not comment, not section, must be key=value pair
            namePtr = tokenPtr;
            tokenPtr = FindChar(tokenPtr, '=');

            if ('=' == *tokenPtr)
            {
                *tokenPtr = '\0';
                namePtr = RemoveTrailingSpace(namePtr);
                if (strcasecmp(namePtr, nnamePtr) == 0)
                {
                    nMatch = true;
                }
                else
                {
                    nMatch = false;
                }
            }
            else
            {
                // Syntax error
                printf("syntax error: expect '=' %s\n", line);
                return -1;
            }
        }

        if ((sMatch == true) && (nMatch == true))
        {
            lMatch = true;
            bzero(line, MAX_LINE);
            newLineSize = snprintf(line, MAX_LINE, "%s = %s\n", nnamePtr, nvaluePtr);
            memmove(positionPtr + newLineSize, bufferPtr, bsize );
            memcpy(positionPtr, line, newLineSize);
            // We are done !
            return osize + newLineSize - lineSize;
        }
    }
    // Scanned to the end of buffer
    if (lMatch == false)
    {
        if (sMatch == false)
        {
            // Didn't find matching section and name
            // add section to the end of buffer and add name/value pair to the new section.
            newLineSize = snprintf(bufferPtr, MAX_LINE,
                                   "\n[%s]\n%s = %s\n",
                                   nsectionPtr, nnamePtr, nvaluePtr);
            return newLineSize + osize;
        }
        else
        {
            // Found matching section which is the last section, but didn't find the name.
            // Add the name/value pair to section end of buffer.
            newLineSize = snprintf(bufferPtr, MAX_LINE, "%s = %s\n", nnamePtr, nvaluePtr);
            return newLineSize + osize;
        }
    }

    return osize;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read and parse configuration file into client configuration data structure.
 *
 * @return
 *  - 0 on succes and dereference of config points to configuration pointer
 *  - -1 on failure and dereference of config points to NULL
 */
//--------------------------------------------------------------------------------------------------
int clientConfigRead
(
    clientConfig_t** configPtr      ///< [IN] Client configuration structure
)
{
    int ret = 0;
    FILE* fPtr = NULL;
    char fname[CLIENT_CONFIG_FILENAME_MAX_LENGTH];

    sprintf(fname, "%s", CLIENT_CONFIG_FILENAME);

    *configPtr = NULL;
    if ((fPtr = fopen(fname,"r")) == NULL)
    {
        printf("%s: failed to open configuration file - %s\n", __func__, fname);
        return -1;
    }

    fseek(fPtr, 0, SEEK_SET);
    ret = ParseConfigFile(fPtr,
                          ConfigurationHandler,
                          (void*)&ClientConfig,
                          LWM2MCORE_OP_READ);
    if (ret == 0)
    {
        *configPtr = &ClientConfig;
    }
    fclose(fPtr);

    return ret;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write one line into configuration file.
 *
 * @return
 *  - Buffer length
 *  - -1 on failure
 */
//--------------------------------------------------------------------------------------------------
int clientConfigWriteOneLine
(
    const char*     sectionPtr,     ///< [IN] Section name
    const char*     namePtr,        ///< [IN] Parameter name
    char*           valuePtr,       ///< [IN] New value
    clientConfig_t* configPtr       ///< [IN] Pointer on client configuration
)
{
    char* bufferPtr = NULL;
    int bsize = MAX_FILE_SIZE;

    bufferPtr = lwm2m_malloc(bsize);
    assert(bufferPtr);
    memset(bufferPtr, 0, bsize);

    if ((bsize = ReadFileToBuffer(bufferPtr)) < 0)
    {
        free(bufferPtr);
        return bsize;
    }

    bsize = WriteConfigLine(bufferPtr, bsize, sectionPtr, namePtr, valuePtr);

    if (bsize > 0)
    {
        memset(bufferPtr + bsize, 0, MAX_FILE_SIZE - bsize);
        bsize = WriteBufferToFile(bufferPtr, (size_t)bsize);
    }

    if (bsize > 0)
    {
        clientConfigRead(&configPtr);
    }

    free(bufferPtr);
    return bsize;
}

//--------------------------------------------------------------------------------------------------
/**
 * Return client configuration pointer
 *
 * @return
 *  - Client configuration pointer
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
clientConfig_t* ClientConfigGet
(
    void
)
{
    return &ClientConfig;
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the client configuration
 */
//--------------------------------------------------------------------------------------------------
void ClientConfigFree
(
    void
)
{
    clientSecurityConfig_t* securityPtr;
    clientSecurityConfig_t* security2Ptr;

    securityPtr = ClientConfig.securityPtr;
    while (securityPtr)
    {
        security2Ptr = securityPtr->nextPtr;
        lwm2m_free(securityPtr);
        securityPtr = security2Ptr;
    }
    ClientConfig.securityPtr = NULL;
}
