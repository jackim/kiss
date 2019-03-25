#pragma once


#include "http.hpp"
#include <vector>

namespace jackim{ namespace kiss{




class WebSocket : public Http
{

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

    virtual void onUpgrade(Request *request);

    virtual void onEstablished();

    virtual void onPing();

    virtual void onTextFrame(std::vector<char> &text , bool fin);
    
    virtual void onBinaryFrame(std::vector<char> &binary , bool fin);

    void onWSData(const char *data , int len);

    FrameType parseFrame(const char *in , int in_len ,
    std::vector<char> &out , int &buflen );

    std::vector<char> _buffer;
    FrameType         _lastType;
};


} }
