#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid!" << endl;
        exit(EXIT_FAILURE);
    }
    const char *ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    signal(SIGINT, resetHandler);
    signal(SIGTERM, resetHandler);
    signal(SIGABRT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    return 0;
}