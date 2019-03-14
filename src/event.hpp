#ifndef KISS_EVENT_HPP
#define KISS_EVENT_HPP

namespace jackim{ namespace kiss{

class Event
{
public:
    virtual void onClose() = 0;
    virtual bool onRead() = 0;
    virtual bool onWrite() = 0;

    
};
}}





#endif //KISS_EVENT_HPP