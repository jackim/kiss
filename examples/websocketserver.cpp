#include <loop.hpp>
#include <websocket.hpp>
#include <server.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

using namespace jackim::kiss;


class Handler:public IWebSocketHandler
{
    public:
    Handler(Base *base)
    {
        _sock = new WebSocket(base , this);
    }

    virtual std::string onEstablished(const std::vector<std::string> &vecProtocol)
    {
        SPDLOG_INFO("test111");
        return "test";
        
    }
    virtual void onTextFrame(const std::vector<char> &text , FrameSeq seq)
    {
        auto data = _sock->makeTextFrame(text , seq);
        _sock->write(data.data() , data.size());
    }

  

    virtual void onCloseFrame()
    {
        SPDLOG_INFO("closeframe");
    }

    virtual void onClose()
    {
        SPDLOG_INFO("close");
    }
    
    private:
    WebSocket *_sock;
   
};


int main()
{
    Loop *loop = new Loop();
    loop->start();
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] %! - %v - %@");

    auto server = new Server(loop);
    server->listen("127.0.0.1" , 8080);
    server->setConnectCB([](Base *base){
        new Handler(base);
    });

    loop->join();
    return 0;
}