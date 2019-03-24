#pragma once

#include <base.hpp>

#include "http-parser/http_parser.h"
#include <map>

namespace jackim{ namespace kiss{
class Request;

class Http
{
public:
    Http(Base *base);
    virtual ~Http(){ free(_parser);}
    virtual void onProcess(Request *request){}
    void close();
    void write(const char *data , int len , std::function<void ()> finish = nullptr);
protected:
    void onData(const char *data , int len);
    bool onClose();

    static int on_message_begin(http_parser*);
    static int on_url(http_parser*, const char *at, size_t length);
    static int on_status(http_parser*, const char *at, size_t length);
    static int on_header_field(http_parser*, const char *at, size_t length);
    static int on_header_value(http_parser*, const char *at, size_t length);
    static int on_headers_complete(http_parser*);
    static int on_body(http_parser*, const char *at, size_t length);
    static int on_message_complete(http_parser*);

    static int on_chunk_header(http_parser*);
    static int on_chunk_complete(http_parser*);

 /* 
  http_cb      on_message_begin;
  http_data_cb on_url;
  http_data_cb on_status;
  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
*/
    Base            *                   _base;
    http_parser_settings                _settings;
    http_parser     *                   _parser = nullptr;
    std::string                         _url;
    std::string                         _filed;
    std::string                         _value;
    bool                                _isFinsh = false;
    std::map<std::string,std::string>   _header;
    std::string                         _body;
    std::map<std::string,std::string>   _query;
};

}}

