#include <iostream>
#include <ftp_mitm.hpp>

int main() {
    FtpMitm mitm;
    mitm.LoadConfig("../config.yml");
    mitm.StartServer();
    mitm.Attack();
}