#pragma once

#include <base.hpp>
#include "http-parser/http_parser.h"

namespace jackim{ namespace kiss{
class Http
{
public:
    Http(Base *base);
    virtual ~Http(){ free(_parser);}
    void onData(const char *data , int len);
    bool onClose();

protected:
    static int on_message_begin(http_parser*);
    static int on_url(http_parser*, const char *at, size_t length);
    static int on_status(http_parser*, const char *at, size_t length);
    static int on_header_field(http_parser*, const char *at, size_t length);
    static int on_header_value(http_parser*, const char *at, size_t length);
    static int on_headers_complete(http_parser*);
    static int on_body(http_parser*, const char *at, size_t length);
    static int on_message_complete(http_parser*);
 
    
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

    http_parser_settings _settings;
    http_parser        * _parser = nullptr;
    std::string         _url;
    std::string         _status;
};

}}

