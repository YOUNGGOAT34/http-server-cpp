#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include<stdexcept>
#include <string>

/*
  This header file handles the following types of errors
  A case where the server fails
  A file I/O fails
  A network error
*/

using string=std::string;

class ServerException: public std::runtime_error{
   public:
     explicit ServerException(const string& error_message) 
         :std::runtime_error("Server Error: "+ error_message){}
};

class FileIOException: public std::runtime_error{
   public:
     explicit FileIOException(const string& error_message)
         :std::runtime_error("File I/O Error: "+ error_message){}
};

class NetworkException: public std::runtime_error{
   public:
      explicit NetworkException(const string& error_message)
               :std::runtime_error("Network Error: "+ error_message){}
};

#endif