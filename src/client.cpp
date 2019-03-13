
#include "common.hpp"
#include "client.hpp"

#include <sstream>

using namespace jackim::kiss;

Client::Client(Loop *loop) : Base(loop)
{

}

void Client::setOpenCB(std::function<void()> open)
{
    _open = open;
}

void Client::connect(const std::string &host ,
 uint16_t port)
{
    std::stringstream ss;
    ss << port;
    std::string service;
    ss >> service;

    addrinfo *result = nullptr;
    auto r = getaddrinfo(host.c_str() , service.c_str() , nullptr , &result);
    if( r == EAI_AGAIN)
    {
       SPDLOG_ERROR("try again later, {}" , gai_strerror(r));
       freeaddrinfo(result);
       return ;
    }
    else if( r != 0)
    {
        SPDLOG_ERROR("errno {} {} " , r , gai_strerror(r));
        freeaddrinfo(result);
        return ;
    }

    auto sock = socket(result->ai_family , result->ai_socktype ,  result->ai_protocol);
    auto flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    int rc = ::connect(sock , result->ai_addr , result->ai_addrlen);
    if(rc == -1 && errno == EINPROGRESS)
    {
        SPDLOG_INFO("connect {} {} in process" , host.c_str() , service.c_str());
        Base::open(sock , EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR);
        return;
    }
    else if( rc == -1)
    {
        SPDLOG_INFO("connect errno {} {}" , errno , strerror(errno));
        return;
    }
    
    Base::open(sock , EPOLLIN | EPOLLRDHUP | EPOLLERR);
    openCBDone();
    freeaddrinfo(result);

}

void Client::openCBDone()
{
    _connected = true;
    if(_open != nullptr)
    {
        _open();
    }
}


bool Client::onWrite()
{
    if(!_connected)
    {
        openCBDone();
        scheduleWrite();
    }
    else
    {
        return Base::onWrite();
    }
    return true;
}