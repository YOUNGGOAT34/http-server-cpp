#ifndef SERVER_H
#define SERVER_H

#include<iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <format>
#include <cerrno>
#include <print>
#include <unistd.h>



//output colors

constexpr auto RED=   "\033[1;31m";
constexpr auto GREEN=   "\033[1;32m";
constexpr auto YELLOW= "\033[1;33m";
constexpr auto RESET ="\033[0m";
constexpr auto WHITE= "\033[1;37m";

using SA =struct sockaddr_in;
constexpr auto PORT =4221;

//signed data types

using i8=char;
using i16=signed short int;
using i32= signed int;
using i64= signed long int;
//unsigned data types
using u8= unsigned char;
using u16=unsigned short int;
using u32= unsigned int;
using u64=unsigned long int;


class Server{
    public:
         enum class STATUS:i32{
            OK=200
         };
    private:
            std::string Version;
            STATUS code;
            std::string body;
            
    public:
            Server(){}
            void start_server(void);
};

void error(const i8 *message);

#endif