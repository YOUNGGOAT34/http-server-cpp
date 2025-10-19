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

#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"
#define WHITE "\033[1;37m"

#define SA struct sockaddr_in
#define PORT 4221

//signed data types
// typedef char i8;
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