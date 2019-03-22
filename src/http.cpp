#include "http.hpp"
#include "http-parser/http_parser.h"

using namespace jackim::kiss;


Http::Http(Base *base)
{
    auto data = std::bind(&Http::onData , this , std::placeholders::_1 , std::placeholders::_2);
    auto close = std::bind(&Http::onClose , this); 
    base->setDataCB(data);
    base->setCloseCB(close);

    _settings.on_message_begin = Http::on_message_begin;
    _settings.on_url = Http::on_url;
    _settings.on_status = Http::on_status;
    _settings.on_header_field = Http::on_header_field;
    _settings.on_header_value = Http::on_header_value;
    _settings.on_headers_complete = Http::on_headers_complete;
    _settings.on_body = Http::on_body;
    _settings.on_message_complete = Http::on_message_complete;

    _parser = (http_parser*)malloc(sizeof(http_parser));
    http_parser_init(_parser, HTTP_REQUEST); 
    _parser->data = this;
}


int Http::on_message_begin(http_parser *_)
{
    return 0;
}

int Http::on_url(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http*)p->data;
    h->_url = std::string(at , at + length);
    return 0;
}

int Http::on_status(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http*)p->data;
    h->_status = std::string(at , at + length);
    return 0;
}

int Http::on_header_field(http_parser *p, const char *at, size_t length)
{
    
    return 0;
}

int Http::on_header_value(http_parser *p, const char *at, size_t length)
{
    
    return 0;
}

int Http::on_body(http_parser *p, const char *at, size_t length)
{
    
    return 0;
}

int Http::on_headers_complete(http_parser *p)
{
    
    return 0;
}


void Http::onData(const char *data , int len)
{
    auto nparsed = http_parser_execute(_parser , &_settings , data , len);
    if(_parser->upgrade)
    {
        
    }
    else if(nparsed != len)
    {
        
    }
}

bool Http::onClose()
{

    return true;
}