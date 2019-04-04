#pragma once
#include <string>
#include <vector>

namespace jackim { namespace kiss{

enum FrameSeq
{
    FRAME_STR,      //  first
    FRAME_CON,      //  continuation
    FRAME_FIN       //  fin
};

class WebSocket;

class IWebSocketHandler
{
public:
    virtual std::string onEstablished(const std::vector<std::string>& vecProtocol) = 0;
    virtual void onPing(const std::vector<char> &data) {};
    virtual void onPong() {};
    virtual void onTextFrame(const std::vector<char> &text , FrameSeq seq) {};
    virtual void onBinaryFrame(const std::vector<char> &binary , FrameSeq seq) {};
    virtual void onError() {};
    virtual void onClose() {};
    virtual void onCloseFrame() {};
};

}}

