#pragma once

#include "common.hpp"
#include "event.hpp"
#include "loop.hpp"
#include <functional>


namespace jackim { namespace kiss{

class Timer : Event
{
public:
    Timer(Loop *loop);
    bool setTimer(uint32_t millsecond , std::function<void ()> timerCB , bool once = false);
    void close();
    virtual ~Timer(){}

protected:

    virtual void onClose(){close();}
    virtual bool onRead();
    virtual bool onWrite(){ return true;}

    Loop*   _loop;
    int     _fd = -1;
    std::function<void ()> _cb;
} ;


}}

