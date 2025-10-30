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
#include<fstream>
#include<filesystem>
#include <thread>



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
using string=std::string;

template<typename K,typename V>
using hashMap=std::unordered_map<K,V>;

template<typename T>
using vector=std::vector<T>;





class Server{
    //public enums and structs
    public:
         enum class STATUS:i32{
            OK=200,
            NOT_FOUND=404,
            CREATED=201,
            INTERNAL_SERVER_ERROR=500,
            BAD_REQUEST=400
         };

         struct  CLIENT_ARGS{
                i32 client_fd;
                i8 *file_path;
         };

    private:
           //private members and methods
            string Version;
            STATUS Code;
            string body;
            string response(STATUS status,const string& __body);
            string extract_request_body_from_path(const string& path );
            string extract_request_body(const string& request);
            vector<string> extract_request_line(const i8 *buffer);
            hashMap<string,string> extract_headers(const i8 *buffer);
            ssize_t user_agent_endpoint(const i32 client_fd,const hashMap<string,string> headers);
            ssize_t echo_endpoint(const string& path,const i32 client_fd);
            ssize_t get_file_endpoint(const i32 client_fd,const string& path);
            ssize_t post_file_endpoint(const string& path,const i32 client_fd);
            i8* read_file_contents(const string &path,size_t& file_size);
            string status_code_to_string(const Server::STATUS code);
            void handle_client(const CLIENT_ARGS& client_args);



            
    public:
            //public methods
            Server(){}
            void start_server(i8 *__directory) ;

};

void error(const i8 *message);
#endif
