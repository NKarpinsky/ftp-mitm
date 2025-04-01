## Simple FTP MITM written on C++ with sockets
### Functionality
- It is a multithreaded library for intercepting and replacing files via FTP.
- The library replaces files for servers and clients specified in the configuration file.
- File replacement is performed using regular expressions.
- Always trying to access request file on server. If it does not exists client will get error.
- Almost all commands are sent to the server unchanged, which reduces the risk of detection.
- It is not a packet filter and should be used in conjuction with iptables.
### Usage
```
#include <ftp-mitm.hpp>

int main() {
    FtpMitm mitm;
    mitm.LoadConfig("config.yml");
    if (mitm.StartServer() >= 0)
        mitm.Attack();
}
```
### Configuration
- YAML configuration file used for initial setup
- Check example configuration in config.yml
- Example:
```
---
config:
  port: <port-for-listening-clients>
  buffer_size:  <transmission-buffer-size>
tasks:
  - client: <client-ip-address>
    server: <server-ip-or-dns>
    substitutions:
      - target: <regex-for-file-replace>
        sub: <path-to-replacement-file>
      ...
```
### Building
- In this repository you can find an example of library client and cmake configuration for integrating the library into your own project.
- The client example demonstrates usage of the library.
