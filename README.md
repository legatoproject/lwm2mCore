Flags to be used
================
`LWM2M_CLIENT_MODE`: enable LWM2M Client interfaces.<br />
`LWM2M_BOOTSTRAP`: enable LWM2M Bootstrap support in a LWM2M Client.<br />
`LWM2M_LITTLE_ENDIAN` or `LWM2M_BIG_ENDIAN`.<br />
`LWM2M_WITH_LOGS` and `WITH_LOGS`: enable logs<br />
`SIERRA`: include Sierra's updates in Wakaama and tinyDTLS.<br />
`WITH_TINYDTLS`: support DTLS<br />
`LWM2M_OLD_CONTENT_FORMAT_SUPPORT`: enable old content-type field value (IANA updates)</br />

Others flags which shall not be used
====================================
`LWM2M_SERVER_MODE`: enable LWM2M Server interfaces.<br />
`LWM2M_BOOTSTRAP_SERVER_MODE`: enable LWM2M Bootstrap Server interfaces.<br />
`LWM2M_SUPPORT_JSON`: enable JSON payload support (implicit when defining `LWM2M_SERVER_MODE`).<br />

Debug flags: do not use in production !!
========================================
`CREDENTIALS_DEBUG`: dump credentials logs.

Some files manage default values for LWM2M resources.<br />
For the moment, several files are available:<br />
examples/linux/device.c: manage some device parameters which can be read by the LWM2M server.<br />
examples/linux/security.c: manage LWM2M credentials.<br />
examples/linux/update.c: manage APIs for package update download and install.<br />

Coding rules
============
See [Legato's C Language Standards](http://legato.io/legato-docs/latest/ccodingStdsMain.html)
