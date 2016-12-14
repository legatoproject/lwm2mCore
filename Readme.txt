Flags to be used:

    LWM2M_CLIENT_MODE to enable LWM2M Client interfaces.
    LWM2M_BOOTSTRAP to enable LWM2M Bootstrap support in a LWM2M Client.
    LWM2M_LITTLE_ENDIAN or LWM2M_BIG_ENDIAN
    LWM2M_WITH_LOGS and WITH_LOGS to enable logs
    SIERRA to include Sierra's updates in Wakaama and tinyDTLS
    WITH_TINYDTLS to support DTLS

Others flags which shall not be used:
    LWM2M_SERVER_MODE to enable LWM2M Server interfaces.
    LWM2M_BOOTSTRAP_SERVER_MODE to enable LWM2M Bootstrap Server interfaces.
    LWM2M_SUPPORT_JSON to enable JSON payload support (implicit when defining LWM2M_SERVER_MODE)


Debug flag to not used in production !!
CREDENTIALS_DEBUG

Some files manage default values for LWM2M resources.
For the moment, 2 files are available:
objectManager/lwm2mcorePortDeviceDefault.c: this files manages some device parameters which can be read by the LWM2M server
objectManager/lwm2mcorePortSecurityDefault.c: this file manages LWM2M credentials
