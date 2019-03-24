#include <loop.hpp>
#include <server.hpp>
#include <request.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

using namespace jackim::kiss;


class Controller: public Http
{
    public:
    Controller(Base *base):Http(base)
    {}

    virtual void onProcess(Request *request)
    {
       std::string str = "HTTP/1.1 200 OK\r\nServer: kiss\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nHello World";
       auto http = request->http;
       bool should_keepalive = request->should_keepalive;
       http->write(str.c_str() , str.length() , [http ,should_keepalive](){
           if(!should_keepalive)
                http->close();
       } );

       delete request;
    }
};


int main()
{
    Loop *loop = new Loop();
    loop->start();

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] %! - %v - %@");

    auto server = new Server(loop);
    server->listen("127.0.0.1" , 8080);
    server->setConnectCB([](Base *base){
       auto controller = new Controller(base);
    });

    loop->join();
    return 0;
}