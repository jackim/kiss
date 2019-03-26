#include "websocket.hpp"
#include "request.hpp"
#include "sha1/sha1.hpp"
using namespace jackim::kiss;


#define SEC_WEBSOCKET_KEY "Sec-WebSocket-Key"
#define SEC_WEBSOCKET_ACCEPT "Sec-WebSocket-Accept"
#define SEC_WEBSOCKET_PROTOCOL "Sec-WebSocket-Protocol"
#define RFC6544_MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WEBSOCKET_HEADER "HTTP/1.1 101 Switching Protocols\r\n" \
"Upgrade: websocket\r\n"    \
"Connection: Upgrade\r\n"

WebSocket::WebSocket(Base *base):Http(base)
{

}

void WebSocket::onUpgrade(Request *request)
{
    
    std::string text;
    text += WEBSOCKET_HEADER;

    if(request->header.count(SEC_WEBSOCKET_KEY) > 0)
    {
        std::string key;
        key += request->header[SEC_WEBSOCKET_KEY];
        key += RFC6544_MAGIC_KEY;
        char base64[SHA1_BASE64_SIZE];
        sha1(key.c_str()).finalize().print_base64(base64);
        text += SEC_WEBSOCKET_ACCEPT;
        text += ": " ;
        text += base64;
        text +="\r\n";
    }

    if(request->header.count(SEC_WEBSOCKET_PROTOCOL) > 0)
    {
        text += SEC_WEBSOCKET_PROTOCOL;
        text += ": " + request->header[SEC_WEBSOCKET_PROTOCOL] + "\r\n";
    }

    text += "\r\n";

    write(text.c_str() , text.length());

    auto data = std::bind(&WebSocket::onWSData , this , std::placeholders::_1 , std::placeholders::_2);
    auto close = std::bind(&WebSocket::onWSClose , this);
    _base->setDataCB(data);
    _base->setCloseCB(close);
    return onEstablished();
}

void WebSocket::onWSData(const char *data , int len)
{
    _buffer.insert(_buffer.end() , data , data + len);
    std::vector<char> out;
    int buflen = 0;
    int parsed = 0;
    do{
        auto type = parseFrame(_buffer.data() + parsed , (int)_buffer.size() ,out , buflen);
        if(type == PING_FRAME)
        {
            onPing();
            parsed += buflen;
        }
        else if(type == INCOMPLETE_FRAME)
        {
            out.assign(_buffer.data() + parsed , _buffer.data() + _buffer.size());
            _buffer = std::move(out);
            return;
        }
        else if(type == INCOMPLETE_TEXT_FRAME){
            onTextFrame(out , FRAME_STR);
            _lastType = type;
            parsed += buflen;
        }
        else if(type == TEXT_FRAME)
        {
            onTextFrame(out , FRAME_FIN);
            parsed += buflen;
        }
        else if(type == INCOMPLETE_BINARY_FRAME)
        {
            onBinaryFrame(out ,FRAME_STR);
            _lastType = type;
            parsed += buflen;
        }
        else if(type == BINARY_FRAME)
        {
            onBinaryFrame(out ,FRAME_FIN);
            parsed += buflen;
        }
        else if(type == CONTINUATION_FRAME){
            if(_lastType == INCOMPLETE_TEXT_FRAME)
            {
                onTextFrame(out , FRAME_CON);
            }
            else
            {
                onBinaryFrame(out , FRAME_CON);
            }
        }
        else if(type == CLOSING_FRAME)
        {
            onCloseFrame();
            close();
            break;
        }
        else
        {
            SPDLOG_ERROR("recv error frame:{0}" , type);
            onError();
            close();
            break;
        }

    }while(parsed < _buffer.size());

    _buffer.clear();

}

WebSocket::FrameType WebSocket::parseFrame(const char *in , int in_len ,
    std::vector<char> &out , int &buflen )
{
    if(in_len < 3)
        return INCOMPLETE_FRAME;
    
    unsigned char *buffer = (unsigned char *)in;

    auto opcode = buffer[0] & 0x0F;
    auto fin = (buffer[0] >> 7) & 0x01;
    auto masked = (buffer[1] >> 7) & 0x01;

    int length = 0;
    int field = buffer[1] & (~0x80);
    int pos = 2;
    unsigned int mask = 0;

    if(field <= 125)
    {
        length = field;
    }
    else if(field == 126)
    {
        length = ((buffer[2] << 8) | buffer[3]);
        pos += 2;
    }
    else if(field == 127)
    {
        length = (
			(buffer[2] << 56) | 
			(buffer[3] << 48) | 
			(buffer[4] << 40) | 
			(buffer[5] << 32) | 
			(buffer[6] << 24) | 
			(buffer[7] << 16) | 
			(buffer[8] << 8) | 
			(buffer[9])
		); 
		pos += 8;
    }

    if(in_len < length + pos)
    {
        return INCOMPLETE_FRAME;
    }

    

    if(masked)
    {
        mask = *((unsigned int*)(buffer+pos));
        pos += 4;

        out.assign(buffer + pos , buffer + pos + length);
        
		for(int i=0; i<length; i++) {
			out[i] = out[i] ^ ((unsigned char*)(&mask))[i%4];
		}
    }
    else
    {
        out.assign(buffer + pos , buffer + pos + length);
    }

    buflen = pos + length;
    


    if (opcode == 0x0) return (fin) ? END_FRAME : CONTINUATION_FRAME;
	else if (opcode == 0x1) return (fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
	else if (opcode == 0x2) return (fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
	else if (opcode == 0x8) return (fin) ? CLOSING_FRAME : ERROR_FRAME;
	else if (opcode == 0x9) return (fin) ? PING_FRAME : ERROR_FRAME;
	else if (opcode == 0xA) return (fin) ? PONG_FRAME : ERROR_FRAME;

}

std::vector<char> WebSocket::makeFrame(FrameType type , const char* data , int len)
{
    std::vector<char> buffer;
    buffer.push_back(type);

    if( len <= 125)
    {
        buffer.push_back(len);
    }
    else if(len <= 0xFFFF)
    {
        buffer.push_back(126);

        buffer.push_back((len >> 8) & 0xFF);
        buffer.push_back(len & 0xFF);
    }
    else{
        buffer.push_back(127);

        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);

        buffer.push_back((len >> 24) & 0xFF);
        buffer.push_back((len >> 16) & 0xFF);
        buffer.push_back((len >> 8) & 0xFF);
        buffer.push_back((len) & 0xFF);
    }

    buffer.insert(buffer.end() , data , data + len);

    return std::move(buffer);
}


std::vector<char> WebSocket::makePongFrame()
{
    return makeFrame(PONG_FRAME , nullptr , 0);
}

std::vector<char> WebSocket::makeTextFrame(const std::vector<char> &text , FrameSeq seq)
{
    if(seq == FRAME_STR)
        return makeFrame(INCOMPLETE_TEXT_FRAME , text.data() , text.size());
    else if(seq == FRAME_CON)
        return makeFrame(CONTINUATION_FRAME , text.data() , text.size());
    else
        return makeFrame(TEXT_FRAME , text.data() , text.size());
}

std::vector<char> WebSocket::makeBinaryFrame(const std::vector<char> &binary , FrameSeq seq)
{
    if(seq == FRAME_STR)
        return makeFrame(INCOMPLETE_BINARY_FRAME , binary.data() , binary.size());
    else if(seq == FRAME_CON)
        return makeFrame(CONTINUATION_FRAME , binary.data() , binary.size());
    else
        return makeFrame(BINARY_FRAME , binary.data() , binary.size());
}
 