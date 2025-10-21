
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



std::string Server::extract_request_body(const std::string& path){

   ssize_t position=path.find_last_of('/');

   return path.substr(position+1);
      
}


std::string Server::extract_user_agent(const std::string& headers){
   return "";
     
}


std::vector<std::string> Server::extract_request_line(const std::string& request){
   std::istringstream raw_request_line(request);
   std::string request_line;

   std::getline(raw_request_line,request_line);

   if(!request_line.empty() && request_line.back()=='\r'){
       request_line.pop_back();
   }

   std::istringstream line_stream(request_line);

   std::string method,path,version;

   line_stream >> method >> path >> version;

   return {method,path,version};
   
}


std::variant<std::string,std::vector<std::string>> Server::tokenize_request(i8 *buffer,REQUEST_TYPE req){
   std::string request(buffer);
   switch (req){

      case REQUEST_TYPE::HEADERS_LINE:
      break;
      case REQUEST_TYPE::REQUEST_LINE:
      return extract_request_line(request);
            
   }
      
    return "";
     
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
   
   
   // STATUS status=path=="/"?STATUS::OK:STATUS::NOT_FOUND;

    
   std::variant<std::string,std::vector<std::string>> tokenized_request=tokenize_request(buffer,REQUEST_TYPE::REQUEST_LINE);
   std::string res;
   if(auto request_ptr=std::get_if<std::vector<std::string>>(&tokenized_request)){
      res=response(STATUS::OK,extract_request_body((*request_ptr)[1]));
   }
    
  
   ssize_t bytes_sent=send(client_fd,res.c_str(),res.size(),0);

   if(bytes_sent==-1){
       error("Failed to send response to client");
   }

   close(server_fd);
}

//HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\nabc

std::string Server::response(STATUS status,const std::string& __body){
      Version="HTTP/1.1";
      Code=status;
      body=__body;


      return std::format(
           "{} {}\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: {}\r\n"
           "\r\n"
           "{}",
           Version,
           status_code_to_string(Code),
           body.size(),
           body   
      );
}

