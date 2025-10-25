
#include "server.hpp"
#include <iostream>



int main(){
   
   std::cout << std::unitbuf; 
   std::cerr << std::unitbuf;

   Server server;
   server.start_server();

   return 0;
}

