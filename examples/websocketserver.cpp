#include <loop.hpp>
#include <websocket.hpp>
#include <server.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

using namespace jackim::kiss;


class Handler: public WebSocket
{
    public:
    Handler(Base *base):WebSocket(base)
    {

    }

    virtual void onEstablished()
    {
        SPDLOG_INFO("established");

    }
    virtual void onTextFrame(std::vector<char> &text , FrameSeq seq)
    {
        auto data = makeTextFrame(text , seq);
        write(data.data() , data.size());
    }

    virtual void onCloseFrame()
    {
        SPDLOG_INFO("closeframe");
    }

    virtual void onClose()
    {
        SPDLOG_INFO("close");
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
       auto handler = new Handler(base);
    });

    loop->join();
    return 0;
}