//
// Created by ht on 17-6-1.
//

#ifndef LIBNET_EXCEPTION_H
#define LIBNET_EXCEPTION_H


#include <exception>
#include <string>

class Exception: public std::exception {
public:
    explicit Exception(const std::string& str);

    explicit Exception(const char *str);

    virtual ~Exception() throw();
    const char* what() const;

private:
    std::string message_;
};

//TODO add backstrace infomation


#endif //LIBNET_EXCEPTION_H
