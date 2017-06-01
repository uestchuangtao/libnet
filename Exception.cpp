//
// Created by ht on 17-6-1.
//

#include "Exception.h"

Exception::Exception(const std::string& str)
        :message_(str)
{

}

Exception::Exception(const char *str)
        :message_(str)
{

}

Exception::~Exception() throw()
{

}

const char* Exception::what() const throw()
{
    return message_.c_str();
}

