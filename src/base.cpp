
#include "common.hpp"
#include "base.hpp"

using namespace jackim::kiss;



Base::Base(Loop *loop ):_loop(loop)
{

}


void Base::setDataCB(std::function<void (const char *data , int len)> data)
{
    _data = data;
}

void Base::setCloseCB(std::function<bool ()> close)
{
    _close = close;
}

void Base::open(Base *base , int fd , uint32_t events)
{
    base->open(fd , events);
}

void Base::open(int fd ,uint32_t events)
{
    _fd = fd;
    _events = events;
    _loop->addEvent(this , _fd , _events);
}

void Base::close(bool del)
{
    if(_fd != -1)
    {
        _loop->delEvent(this , _fd , _events);
        ::close(_fd);
        if(del)
            delete this;
    }
}

void Base::onClose()
{
    bool del = true;
    if(_close != nullptr)
        del = _close();
    close(del);
}

bool Base::onRead()
{
    auto buf = _loop->getBuf();
    auto count = recv(_fd , buf.data , buf.len , 0);
    if(count == 0)
    {
        SPDLOG_WARN("recv fd %d close" , count);
        return false;
    }
    else if(count == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return true;
        }
        SPDLOG_WARN("recv fd %d errno %d %s" , _fd , errno ,strerror(errno));
        return false;
    }
    else{
        _data(buf.data , count);
    }
    return true;

}

void Base::scheduleWrite()
{
    _events ^= EPOLLOUT;
    _loop->modEvent(this , _fd ,  _events );
}

bool Base::onWrite()
{
    if(_queue.empty())
    {
        SPDLOG_ERROR("empty writebuffer");
        return false;
    }

    while(!_queue.empty())
    {
        auto  wbuf = _queue.front();
        auto count = send(_fd , wbuf.buf.data + wbuf.pos , wbuf.buf.len - wbuf.pos ,0 );
        if(count == wbuf.buf.len - wbuf.pos)
        {
            if(wbuf.finish != nullptr)
            {
                wbuf.finish();
            }
            delete wbuf.buf.data;
            _queue.pop();
        }
        else if(count >= 0)
        {
            wbuf.pos += count;
            return true;
        }
        else 
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                return true;
            }
            else
            {
                SPDLOG_ERROR("this fd %d send errno %d %s" , _fd , errno , strerror(errno));
                return false;
            }
            
        }
    }
    scheduleWrite();
    return true;

}

void Base::write(const char *data , int len , std::function<void ()> finish)
{
    auto count = send(_fd , data , len ,0 );
    int left = 0;
    char *buf = nullptr;
    if( count == len)
    {
        if(finish != nullptr)
        {
            finish();
        }
        return ;
    }
    else if(count >= 0)
    {
        left = len - count;
        buf = new char[left];
        memcpy(buf , data + count , left);
    }
    else{
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                left = count;
                buf = new char[left];
                memcpy(buf , data  , left);
            }
            else
            {
                SPDLOG_ERROR("this fd {} send errno {} {}" , _fd , errno , strerror(errno));
                return ;
            }
    }
    WriteBuf wbuf;
    wbuf.buf.data = buf;
    wbuf.buf.len = left;
    wbuf.finish = finish;
    _queue.push(wbuf);
    scheduleWrite();

}


