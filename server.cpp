
#include "server.h"

void error(const i8 *message){
   //   std::cerr<<std::format("{}\n",message);

   std::print(std::cerr,"ERROR: {} ({})\n",message,strerror(errno));

   exit(EXIT_FAILURE);
}

void server(void){
   
   i32 socketFd=socket(AF_INET,SOCK_STREAM,0);

   if(socketFd<0){
      error("socket FD creation error");
   }

}