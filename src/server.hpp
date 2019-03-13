#pragma once

#include <functional>

#include "base.hpp"
#include "loop.hpp"




#define MAX_BACKLOG 1024

namespace jackim { namespace kiss{

class Server : Base
{
    
public:
    Server(Loop *loop):Base(loop){}

    void listen(const std::string &addr , uint16_t port);
   
    virtual bool onRead();

    void setConnectCB(std::function<void (Base *base)> connect);

    void close()
    {
        Base::close(false);
    }


private:
    std::function<void (Base *base)> _connect = nullptr;
};

}}
