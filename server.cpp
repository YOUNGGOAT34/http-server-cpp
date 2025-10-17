
#include "server.h"


void error(const i8 *message){
   
   std::print(std::cerr,RED"ERROR: {} ({})\n" RESET,message,strerror(errno));

   exit(EXIT_FAILURE);
}

void server(void){
   
   i32 server_fd=socket(AF_INET,SOCK_STREAM,0);

   if(server_fd<0){
      error("socket FD creation error");
   }

   i32 reuse=1;

   if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0){
      error("Setting the address to be reusable failed");
   }

   SA server_address;

   server_address.sin_family=AF_INET;
   server_address.sin_addr.s_addr=INADDR_ANY;
   server_address.sin_port=htons(PORT);

   if(bind(server_fd,(const struct sockaddr *)&server_address,sizeof(server_address))!=0){
       error("Failed to bind the server file descriptor to this address");
   }

   i32 connection_backlog=50;
   if(listen(server_fd,connection_backlog)!=0){
       error("Failed to listen on PORT 4221");
   }

   SA client_address;
   socklen_t client_address_size=sizeof(client_address);

   std::cout<<WHITE"Waiting for client connection" RESET<<std::endl;

   i32 client_fd=accept(server_fd,(struct sockaddr *)&client_address,&client_address_size);

   if(client_fd==-1){
       error("Failed to accept connection");
   }

   std::cout<<"Accepted connection"<<std::endl;

   ssize_t bytes_sent=send(client_fd,"HTTP/1.1 200 ok\r\n\r\n",23,0);

   if(bytes_sent==-1){
       error("Failed to send response to client")
   }

   close(server_fd);

}