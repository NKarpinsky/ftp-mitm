#include <iostream>
#include <ftp-mitm.hpp>

int main() {
    FtpMitm mitm;
    mitm.LoadConfig("../config.yml");
    mitm.StartServer();
    mitm.Attack();
}