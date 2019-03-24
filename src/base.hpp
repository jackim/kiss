#pragma once

#include <functional>
#include <queue>
#include <mutex>

#include "event.hpp"
#include "loop.hpp"





namespace jackim { namespace kiss{
class Base : Event 
{
public:
    Base(Loop *loop);

    void setDataCB(std::function<void (const char *data , int len)> data);
    void setCloseCB(std::function<bool ()> close);
    void close(bool del = true);
    void write(const char *data , int len , std::function<void ()> finish = nullptr);
    virtual ~Base() {};
    Loop* getLoop()
    {
        return _loop;
    }
protected:
    static void open(Base *base , int fd , uint32_t events);
    void open(int fd , uint32_t events);
    virtual void onClose();
    virtual bool onRead();
    virtual bool onWrite();
    void scheduleWrite();
    int getSock() 
    { 
        return _fd;
    }

private:

    struct WriteBuf
    {
        Buf buf;
        std::function<void ()> finish;
        int pos;
    };
    Loop*   _loop;
    int     _fd = -1;
    std::queue<WriteBuf> _queue;
    std::recursive_mutex  _mutex;
    int     _events;
    std::function<void (const char *data , int len)> _data = nullptr;
    std::function<bool ()>                           _close = nullptr;
};



}}
