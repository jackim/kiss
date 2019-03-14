#include "loop.hpp"


using namespace jackim::kiss;

Loop::Loop(int buflen , int timeout) : _timeout(timeout)
{
   _buf = {new char[buflen] , buflen };
   _epfd = epoll_create(1);
   if(_epfd == -1)
   {
       SPDLOG_ERROR("epoll_create {} {}" , errno , strerror(errno));
   }
}

Loop::~Loop()
{
    stop();
    close(_epfd);
    delete[] _buf.data;
}

void Loop::start()
{
    _flag = true;
    _thd = std::thread([&]{
        do{
            if(!poll(_timeout))
                break;
        }while(_flag);
        
    });

    _thd.get_id();


}

void Loop::stop()
{
    if(_flag)
    {
        _flag = false;
    }
}

void Loop::join()
{
    if(_flag)
    {
        _thd.join();
    }
}



bool Loop::poll(int timeout)
{
    auto count = epoll_wait(_epfd , _events , 
    sizeof(_events)/sizeof(_events[0]) ,timeout); 
    if(count == 0) // timeout
    {
        
    }
    else if(count == -1)
    {
        if(errno != EINTR)
        {
            SPDLOG_ERROR("epoll_wait errno {} {}" , errno , strerror(errno));
            return false; 
        }  
        
    }
    else{
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        for( auto i = 0 ; i < count ; i++)
        {
            auto fd = _events[i].data.fd;
            auto events = _events[i].events;

            if(_map.count(fd) <= 0)
            {
                SPDLOG_ERROR("this fd {} isn't exist in map " , fd);
                return false;
            }

            auto e = _map[fd];
            if(events & EPOLLRDHUP)
            {
                e->onClose();
                continue;
            }

            if(events & EPOLLERR)
            {
                int opt;
                socklen_t len = sizeof(opt);
                getsockopt(fd , SOL_SOCKET , SO_ERROR , &opt , &len);
                SPDLOG_WARN("this fd {} occur errno %d %s " , opt , strerror(opt));
                e->onClose();
                continue;
            }
            
            if(events & EPOLLOUT)
            {
                if(!e->onWrite())
                {    
                    e->onClose();
                    continue;
                }
            }

            if( events & EPOLLIN)
            {
                if(!e->onRead())
                {    
                    e->onClose();
                    continue;
                }
            }

        }

    }
     return true;
    
}

bool Loop::addEvent(Event *e , int fd ,  uint32_t events)
{
    {
        std::lock_guard<std::recursive_mutex> guard(_mutex); 
        if(_map.count(fd) > 0)
        {
            SPDLOG_WARN("addEvent this fd {} already in map!" , fd);
            return false;
        }
        _map[fd] = e;
    }

    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(_epfd , EPOLL_CTL_ADD  , fd , &ev) >= 0;
}

 bool Loop::modEvent(Event *e , int fd , uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(_epfd , EPOLL_CTL_MOD , fd , &ev) >= 0;
}

bool Loop::delEvent(Event * e , int fd ,  uint32_t )
{
    {
        std::lock_guard<std::recursive_mutex> guard(_mutex); 
        if(_map.count(fd) == 0)
        {
            SPDLOG_WARN("delEvent thid fd {} isn't exist in map" , fd);
            return false;
        }
        _map.erase(fd);
    }

    epoll_event ev;
    ev.data.ptr = e;
    return epoll_ctl(_epfd , EPOLL_CTL_DEL , fd , &ev) >= 0;
}

Buf Loop::getBuf()
{
    return _buf;
}

