#include "server.hpp"

#include <string.h>
#include <sstream>

using namespace jackim::kiss;

 void Server::listen(const std::string &addr , uint16_t port)
 {
    std::stringstream ss;
    ss << port;
    std::string service;
    ss >> service;
    addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    
    addrinfo *result = nullptr;
    auto r = getaddrinfo(addr.c_str() , service.c_str() , nullptr , &result);
    if( r != 0)
    {
        SPDLOG_ERROR("getaddrinfo error");
        return ;
    }
    auto sock = socket(result->ai_family , result->ai_socktype , result->ai_protocol);
    int reuse = 1;
    socklen_t len = sizeof(reuse);
    setsockopt(sock , SOL_SOCKET , SO_REUSEADDR , &reuse , len);
    bind(sock , result->ai_addr , result->ai_addrlen);

    auto flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    ::listen(sock , MAX_BACKLOG);

    Base::open(sock , EPOLLIN | EPOLLRDHUP | EPOLLERR);
    
}

void Server::setConnectCB(std::function<void (Base *base)> connect)
{
    _connect = connect;
}

bool Server::onRead()
{
    do{
        auto sock = Base::getSock();
        sockaddr addr;
        socklen_t len;
        auto f = accept4(sock , &addr , &len , SOCK_NONBLOCK);
        if(f > 0 )
        {   
            auto base = new Base(Base::getLoop());
            Base::open(base, f , EPOLLIN | EPOLLRDHUP | EPOLLERR);
            if(_connect != nullptr)
                _connect(base);
        }
        else 
        {
            if( errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            SPDLOG_ERROR("accept4 errno %d %s" , errno , strerror(errno));
            return false;
        }
        
    }while(1);
    return true;
}