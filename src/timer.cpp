#include "timer.hpp"


using namespace jackim::kiss;



Timer::Timer(Loop *loop):_loop(loop)
{
    _fd = timerfd_create(CLOCK_MONOTONIC , TFD_NONBLOCK);
    _loop->addEvent(this , _fd , EPOLLIN);
}


bool Timer::setTimer(uint32_t millsecond , std::function<void ()> timerCB , bool once)
{
    itimerspec new_value;
    timespec spec;
    
    spec.tv_sec = millsecond / 1000;
    spec.tv_nsec = ( millsecond % 1000) * 1000 * 1000;

    new_value.it_value = spec;
    if(once)
    {
        new_value.it_interval = {0 , 0};
    }    
    else
    {
        new_value.it_interval = spec;
    }

    _cb = timerCB;

    if(timerfd_settime(_fd , 0 , &new_value , nullptr) != 0)
    {
        SPDLOG_ERROR("{} {}" , errno , strerror(errno));
        return false;
    }

    return true;

}

void Timer::close()
{
    if(_fd != -1)
    {
        _loop->delEvent(this , _fd , 0);
        ::close(_fd);
        _fd = -1;
    }
}

bool Timer::onRead()
{
    uint64_t count;
    auto size = ::read(_fd , &count , sizeof(count));
    if( size == -1 )
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return true;
        }
        
    }
    else if(size == sizeof(count))
    {
        for(auto i = 0 ; i < count ; i++)
            _cb();
        return true;
    }
    
    
    SPDLOG_ERROR("{} {}" , errno , strerror(errno));
    return false;
}