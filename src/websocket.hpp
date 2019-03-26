#pragma once


#include "http.hpp"
#include <vector>

namespace jackim{ namespace kiss{

enum FrameSeq
{
    FRAME_STR,      //  first
    FRAME_CON,      //  continuation
    FRAME_FIN       //  fin
};


class WebSocket : public Http
{

public:
    WebSocket(Base *base);
    virtual void onEstablished(){}
    virtual void onPing(){}
    virtual void onTextFrame(std::vector<char> &text , FrameSeq seq){}
    virtual void onBinaryFrame(std::vector<char> &binary , FrameSeq seq){}
    virtual void onError(){}
    virtual void onClose(){}
    virtual void onCloseFrame(){}
    std::vector<char> makePongFrame();
    std::vector<char> makeTextFrame(const std::vector<char> &text , FrameSeq seq);
    std::vector<char> makeBinaryFrame(const std::vector<char> &binary , FrameSeq seq);
    virtual ~WebSocket(){SPDLOG_INFO("1");}
protected:

    enum FrameType
    {
        ERROR_FRAME=0xFF00,
        INCOMPLETE_FRAME=0xFE00,

        INCOMPLETE_TEXT_FRAME=0x01,
        INCOMPLETE_BINARY_FRAME=0x02,
        CONTINUATION_FRAME = 0x00,
		END_FRAME = 0x80,
        TEXT_FRAME=0x81,
        BINARY_FRAME=0x82,
        CLOSING_FRAME=0x88,
        PING_FRAME=0x19,
        PONG_FRAME=0x1A
    };
    std::vector<char> makeFrame(FrameType type , const char *data , int len);
    virtual void onUpgrade(Request *request);
    void onWSData(const  char *data , int len);
    bool onWSClose(){ onClose(); return Http::onClose(); }
    FrameType parseFrame(const  char *in , int in_len ,
    std::vector< char> &out , int &buflen );

    
    std::vector<char> _buffer;
    FrameType         _lastType;
};


} }
