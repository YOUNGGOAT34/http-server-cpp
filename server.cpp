
#include "server.hpp"

//echo end point
//user agent end point


void error(const i8 *message){
   std::print(std::cerr, "{} ERROR: {} ({}){}\n" ,RED,message,strerror(errno),RESET);
   exit(EXIT_FAILURE);
}

std::string Server::status_code_to_string(Server::STATUS code){
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


ssize_t Server::user_agent_endpoint(i32 client_fd,std::unordered_map<std::string,std::string> headers){

   if(headers.find("User-Agent")==headers.end()){
       error("User-Agent not found in the headers");
   }

   std::string res=response(STATUS::OK,headers["User-Agent"]);
   return send(client_fd,res.c_str(),res.size(),0);
}


std::vector<std::string> Server::extract_request_line(i8 *buffer){
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

   return {method,path,version};
   
}


std::unordered_map<std::string,std::string> Server::extract_headers(i8 *buffer){
       
   std::unordered_map<std::string,std::string> headers;
       
       std::istringstream buffer_stream(buffer);
       std::string line;

       bool is_request_line=true;

       while(std::getline(buffer_stream,line)){

         if(!line.empty() && line.back()=='\r'){
             line.pop_back();
         }
        
         if(is_request_line){
            is_request_line=false;
            continue;
         }

         if(line.empty()){
            break;
         }
         
         size_t colon_position=line.find(':');

         if(colon_position!=std::string::npos){
          
            std::string key=line.substr(0,colon_position);
            std::string value=line.substr(colon_position+1);

            /*
               Header is usually something like Host: localhost:port \r\n
               right now we have trimmed the crlf

               now we have the Host as the key ,and the localhost and port and the value ,but the value might
               contain some spaces at the start and we shoulheaders[key]=value;d get rid of that

            */

            size_t first_non_space_position=value.find_first_not_of(" \t");
            if(first_non_space_position!=std::string::npos){
                value=value.substr(first_non_space_position);
            }else{
               value.clear();
            }


            headers[key]=value;

         }

       }

       return headers;
    
}



ssize_t Server::echo_endpoint(std::string path,i32 client_fd){
   std::string res=response(STATUS::OK,extract_request_body(path));
   return send(client_fd,res.c_str(),res.size(),0);
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

   while(1){

      i32 client_fd=accept(server_fd,(struct sockaddr *)&client_address,&client_address_size);
   
      if(client_fd==-1){
          error("Failed to accept connection");
      }
   
      std::cout<<"Accepted connection\n"
  

      std::thread(&Server::handle_client,this,client_fd).detach();
        
   }


   close(server_fd);
}

void Server::handle_client(i32 client_fd){

  
      i8 buffer[BUFFER_SIZE]={0};
   
      ssize_t received_bytes=recv(client_fd,buffer,BUFFER_SIZE-1,0);
      if(received_bytes<0){
           error("Error receiving client request");
      }
      buffer[received_bytes]='\0';
      
      std::vector<std::string> request_line=extract_request_line(buffer);
      std::string path=request_line[1];
     
      ssize_t bytes_sent;
   
      if(path.starts_with("/echo/")){
   
         bytes_sent=echo_endpoint(path,client_fd);
      }else if(path.starts_with("/user-agent")){
          std::unordered_map<std::string,std::string> headers=extract_headers(buffer);
          bytes_sent=user_agent_endpoint(client_fd,headers);
      }
       
      if(bytes_sent==-1){
          error("Failed to send response to client");
      }

      shutdown(client_fd,SHUT_WR);

      close(client_fd);
    
}

std::string Server::response(STATUS status,const std::string& __body){
      std::string version="HTTP/1.1";
      return std::format(
           "{} {}\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: {}\r\n"
           "\r\n"
           "{}",
           version,
           status_code_to_string(status),
           __body.size(),
           __body   
      );
}

