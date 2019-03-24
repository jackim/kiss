#pragma once
#include "http.hpp"
#include <string>
#include <map>
namespace jackim { namespace kiss{

struct Request
{
    Http        *http;

    std::string url;
    std::string path;
    std::string query;
    
    std::map<std::string , std::string> header;
    std::string body;
    unsigned short http_major;
    unsigned short http_minor;

    std::string method;

    bool    should_keepalive;




};

}}
