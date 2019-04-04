#include "utils.hpp"

#include <vector>
#include <string>
#include <sstream>


 namespace jackim { namespace kiss{

std::vector<std::string> split(const std::string& s , char flag ) 
{
    std::istringstream iss(s);
    std::vector<std::string> vec;
    std::string temp;
    while (getline(iss, temp, flag)) {
        vec.push_back(temp);
    }
    return vec;
}


}}

