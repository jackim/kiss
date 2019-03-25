#include "http.hpp"
#include "http-parser/http_parser.h"
#include "request.hpp"

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
    _settings.on_chunk_header = Http::on_chunk_header;
    _settings.on_chunk_complete = Http::on_chunk_complete;

    _parser = (http_parser*)malloc(sizeof(http_parser));
    http_parser_init(_parser, HTTP_REQUEST); 
    _parser->data = this;
    _base = base;
}

 bool Http::onClose()
 {
     delete this;
     return true;
 }

 void Http::close()
 {
    _base->close();
    delete this;
 }

void Http::write(const char *data , int len , std::function<void ()> finish )
{
    _base->write(data , len , finish);
}

int Http::on_message_begin(http_parser *p)
{
    Http *h = (Http*)p->data;
    return 0;
}

int Http::on_url(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http*)p->data;
    h->_url += std::string(at , at + length);
    return 0;
}

int Http::on_status(http_parser *p, const char *at, size_t length)
{
    return 0;
}

int Http::on_header_field(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http *)p->data;
    if(h->_value != "")
    {
        h->_header[h->_filed] = h->_value;
        h->_value = "";
        h->_filed = "";
    }

    h->_filed += std::string(at , at + length );
    return 0;
}

int Http::on_header_value(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http *)p->data;
    h->_value += std::string(at , at + length);
    return 0;
}


int Http::on_body(http_parser *p, const char *at, size_t length)
{
    Http *h = (Http *)p->data;
    h->_body += std::string(at , at + length);
    return 0;
}

int Http::on_headers_complete(http_parser *p)
{
    Http *h = (Http *)p->data;
    h->_header[h->_filed] = h->_value;
    h->_value = "";
    h->_filed = "";
    return 0;
}

int Http::on_message_complete(http_parser*p)
{
    Http *h = (Http *)p->data;
    h->_isFinsh = true;
    return 0;
}

int Http:: on_chunk_header(http_parser*)
{
    return 0;
}

int Http:: on_chunk_complete(http_parser*)
{
    return 0;
}

void Http::onData(const char *data , int len)
{
    if(_isFinsh)
    {
        http_parser_init(_parser, HTTP_REQUEST);
        _isFinsh = false;
    }

    auto nparsed = http_parser_execute(_parser , &_settings , data , len);
    if(_parser->upgrade)
    {
        
    }
    else if(nparsed != len)
    {
        SPDLOG_ERROR("parser error {0}" ,  std::string(data , data + len));
        _base->close(true);        
        return;
    }

    if(_isFinsh)
    {
        auto request = new Request();

        request->http = this;
        http_parser_url u;
        http_parser_url_init(&u);
        http_parser_parse_url(_url.c_str() , _url.length() , true , &u);

        #define REQ(X , E)  if(u.field_set & (1 << E)) \
        X = std::string(_url.c_str() + u.field_data[E].off ,\
         _url.c_str() + u.field_data[E].off + u.field_data[E].len)
       
        REQ(request->path , UF_PATH);
        REQ(request->query , UF_QUERY);

        request->url = std::move(_url);
        _url = "";
        request->header = std::move(_header);
        _header = std::map<std::string ,std::string>();
        request->body = std::move(_body);
        _body = "";
        request->http_major = _parser->http_major;
        request->http_minor = _parser->http_minor;
        request->method = http_method_str((http_method)_parser->method);
        request->should_keepalive = http_should_keep_alive(_parser);


        onProcess(request);
    }

}

