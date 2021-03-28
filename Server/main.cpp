#include <iostream>
#include "tcpServer.h"
int main() {
    tcpServer *server = new tcpServer(54000);
    server->start();
    return 0;
}
