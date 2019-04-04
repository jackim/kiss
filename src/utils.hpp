#pragma once

#include <vector>
#include <string>

namespace jackim { namespace kiss{

struct Buf
{
    char *  data;
    int     len;
};

std::vector<std::string> split(const std::string& s , char flag );

}}




