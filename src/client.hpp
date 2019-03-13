#include "base.hpp"

namespace jackim{ namespace kiss{
    
class Client :public Base
{
public:
    Client(Loop *loop);
    void setOpenCB(std::function<void()> open);
    void connect(const std::string &host , uint16_t port);

    virtual bool onWrite();

private:
    void openCBDone();

    std::function<void()>   _open = nullptr;
    bool                    _connected = false;
};

}}

