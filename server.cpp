
#include "server.hpp"


void error(const i8 *message){
   
   std::print(std::cerr, "{} ERROR: {} ({}){}\n" ,RED,message,strerror(errno),RESET);

   exit(EXIT_FAILURE);
}

std::string status_code_to_string(Server::STATUS code){
   switch(code){
        case Server::STATUS::OK:
               return "200 OK";
        case Server::STATUS::NOT_FOUND:
               return "404 NOT FOUND";
        default: return "500 Unknown";

   }
    
} 


void Server::start_server(void){

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

   std::cout<< WHITE << "Waiting for client connection" <<RESET <<std::endl;

   i32 client_fd=accept(server_fd,(struct sockaddr *)&client_address,&client_address_size);

   if(client_fd==-1){
       error("Failed to accept connection");
   }

   std::cout<<"Accepted connection"<<std::endl;

    i8 buffer[BUFFER_SIZE]={0};

  

   ssize_t received_bytes=recv(client_fd,buffer,BUFFER_SIZE-1,0);
   if(received_bytes<0){
        error("Error receiving client request");
   }
   buffer[received_bytes]='\0';
   std::string request(buffer);

   std::istringstream raw_request_line(request);
   std::string request_line;
   std::getline(raw_request_line,request_line);

   if(!request_line.empty() && request_line.back()=='\r'){
       request_line.pop_back();
   }

   std::istringstream line_stream(request_line);

   std::string method,path,version;

   line_stream >> method >> path >> version;
   STATUS status=path=="/"?STATUS::OK:STATUS::NOT_FOUND;

   std::string res=response(status);
   ssize_t bytes_sent=send(client_fd,res.c_str(),res.size(),0);



   if(bytes_sent==-1){
       error("Failed to send response to client");
   }

   close(server_fd);
}

std::string Server::response(STATUS status){
      Version="HTTP/1.1";
      Code=status;

      return std::format(
           "{} {}\r\n"
           "\r\n",
           Version,
           status_code_to_string(Code)   
      );
}

