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
#include <sstream>
#include<unordered_map>
#include <vector>
#include <variant>
#include <unistd.h>



//output colors

constexpr auto RED=   "\033[1;31m";
constexpr auto GREEN=   "\033[1;32m";
constexpr auto YELLOW= "\033[1;33m";
constexpr auto RESET ="\033[0m";
constexpr auto WHITE= "\033[1;37m";

using SA =struct sockaddr_in;
constexpr auto PORT =4221;
constexpr auto BUFFER_SIZE=4096;

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
            OK=200,
            NOT_FOUND=404
         };

         enum class REQUEST_TYPE{
              REQUEST_LINE,
              HEADERS_LINE 
         };

    private:
            std::string Version;
            STATUS Code;
            std::string body;
            std::string response(STATUS status,const std::string& __body);
            std::string extract_request_body(const std::string& path );
            std::variant<std::string,std::vector<std::string>> tokenize_request(i8 *buffer,REQUEST_TYPE req);
            std::vector<std::string> extract_request_line(const std::string& request);
            std::string user_agent_endpoint(std::vector<std::string,std::string> headers);
            std::unordered_map<std::string,std::string> extract_headers(i8 *buffer);
            ssize_t echo_endpoint(std::string path,i32 client_fd);


            
    public:
            Server(){}
            void start_server(void);

};

void error(const i8 *message);
std::string status_code_to_string(Server::STATUS code);

#endif