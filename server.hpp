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
typedef char i8;
typedef signed short int i16;
typedef signed int i32;
typedef signed long int i64;
//unsigned data types
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;


class Server{
    public:
            Server(){}
            void error(const i8 *message);
            void start_server();
};

void server(void);

#endif