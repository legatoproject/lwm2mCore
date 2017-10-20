How to launch Lw2MCore Linux client
================
Advice: Create a `build` directory in `examples/linux` directory and go to the build directory with
`cd build`

1. `cmake ..`
2. `make`
3. Copy the `configClient.txt` file from `examples/linux` directory to `build` directory.
This means that the `configClient.txt` in `examples/linux` directory should not be updated.
4. Update the `configClient.txt` file according to the server on which the client will connect
5. Launch the client `./lwm2mcoreclient [-d]`
    * `-d` option activates DTLS debug logs

Remarks
================
1. Only secured connection is supported, using PSK security level

Tips
================
1. In case of any connection issue, removing `config0.txt`, `config0.bak` files and launching again
the client will initiate a new connection to the bootstrap server filled in the `configClient.txt`
file.

Connection to the default server
================
The default server indicated in the `configClient.txt` file from `examples/linux` directory is
the Leshan bootstrap server (<http://leshan.eclipse.org/bs/>).
To be sure that the client can connect to the Leshan bootstrap server, please follow this procedure:

1. Go to <http://leshan.eclipse.org/bs/>
2. Search if the client `ENDPOINT` indicated in the `configClient.txt` file is present in the list.
    * If the client is in the list, check if the configuration is correct.
    * If the client is not in the list, use the following API to create it using a REST client:
        * `POST` command on <http://leshan.eclipse.org/bs/api/bootstrap/#endpoint#> where
        `#endpoint#` is the `ENDPOINT` parameter filled in the `configClient.txt` file.
        * Set the `Content-Type` field to `application/json`
        * Data:

        ```
        {
            "servers": {
                "1": {
                    "shortId": "1"
                }
            },
            "security": {
                "0": {
                    "uri": "coaps://leshan.eclipse.org:5784", <!-- Bootstrap server address -->
                    "bootstrapServer": true,
                    "securityMode": "PSK",
                    "publicKeyOrId": [49, 49, 49, 49, 49], <!-- Set DEVICE PKID value -->
                    "secretKey": [50, 50, 50, 50, 50],     <!-- Set SECRET KEY value -->
                    "serverId": "0"
                },
                "1": {
                    "uri": "coaps://leshan.eclipse.org:5684", <!-- DM server address -->
                    "securityMode": "PSK",
                    "publicKeyOrId": [50, 50, 50, 50, 50], <!-- Set any value -->
                    "secretKey": [50, 50, 50, 50, 50],     <!-- Set any value -->
                    "serverId": "1"
                }
            }
        }
        ```
        In this example, `publicKeyOrId` and `secretKey` are in decimal format.

3. Go to <http://leshan.eclipse.org/#/clients>
4. Search if the client `ENDPOINT` is present in the list:
    * If the client is in the list, check if the configuration is correct.
    Using data indicated above: Identity: `22222`, Key: `3232323232` (key is always displayed in
    hexadecimal format).
    * If the client is not in the list, create it using the Leshan server interface by filling
    the correct configuration.
