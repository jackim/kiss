#ifndef KISS_LOOP_HPP
#define KISS_LOOP_HPP

#include <unordered_map>
#include <mutex>
#include "common.hpp"
#include "event.hpp"
#include "buf.hpp"

#define MAX_EVENT_NUM  256
#define MAX_BUFFER_LENGTH 4096

namespace jackim{ namespace kiss{

class Loop
{
public:
    Loop(int buflen = MAX_BUFFER_LENGTH ,  int timeout = -1);
    virtual ~Loop();

    bool poll(int timeout = -1);
   
    void start();

    void stop();

    bool addEvent(Event *e , int fd ,   uint32_t events);

    bool modEvent(Event *e , int fd , uint32_t events);

    bool delEvent(Event * e , int fd , uint32_t );

    Buf getBuf();
    
    

private:
    int             _epfd;
    epoll_event     _events[MAX_EVENT_NUM];
    bool            _flag;
    int             _timeout;
    std::thread     _thd;
    std::unordered_map<int , Event *> _map;
    Buf             _buf;
    std::recursive_mutex      _mutex;
};
}}

#endif //KISS_LOOP_HPP