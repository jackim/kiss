#include <loop.hpp>
#include <base.hpp>
#include <server.hpp>
#include <client.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

using namespace jackim::kiss;




int main()
{
    Loop *loop = new Loop();
    loop->start();

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] %! - %v - %@");

    auto server = new Server(loop);
    server->listen("127.0.0.1" , 8080);
    server->setConnectCB([](Base *base){
        base->setDataCB([base](const char *data , int len){
            base->write(data , len);
        });
    });
    auto client = new Client(loop);
    client->setOpenCB([]{
        SPDLOG_INFO("connected to server!");
    });
    client->setDataCB([](const char *data , int len){
        std::string str(data , data + len);
        SPDLOG_INFO(str);
    });
    client->connect("127.0.0.1" , 8080);
    std::string line;
    while(getline(std::cin , line))
    {
        if(line == "exit")
            break;
        client->write(line.c_str() , line.length() );
    }
    return 0;
}