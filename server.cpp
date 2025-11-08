
#include "server.hpp"


void Server::error(const i8 *message){
   std::print(std::cerr, "{} ERROR: {} ({}){}\n" ,RED,message,strerror(errno),RESET);
   // exit(EXIT_FAILURE);
}

string Server::status_code_to_string(const Server::STATUS code){
   switch(code){
        case Server::STATUS::OK:
               return "200 OK";
        case Server::STATUS::CREATED:
               return "201 CREATED";
        case Server::STATUS::NOT_FOUND:
               return "404 NOT FOUND";
        case Server::STATUS::INTERNAL_SERVER_ERROR:
               return "500 INTERNAL SERVER ERROR";
        default: return "500 Unknown";
   }
    
}


//header manipulation method
string Server::get_header_value(string request,HEADERS header){
     
      string header_type;

      switch(header){
         case HEADERS::CONTENT_LEN:
               header_type="Content-Length";
               break;

         case HEADERS::CONNECTION:
               header_type="Connection";
               break;

         default:
               break;
      }
      
      hashMap<string,string> headers=extract_headers(request.c_str());

      if(headers.find(header_type)==headers.end()){
         return "";
       }
      
       return headers.at(header_type);
}

/*
   When used for the echo endpoint 
   /echo/body
   supposed to return the {body}
*/

string Server::extract_request_body_from_path(const string& path){
   ssize_t position=path.find_last_of('/');
   return path.substr(position+1);
}

/*
  Extracting the request body from the request buffer
*/

string Server::extract_request_body(const string& request){
        size_t pos=request.find("\r\n\r\n");
        if(pos==std::string::npos) return "";

        return request.substr(pos+4);
}

//endpoint methods

ssize_t Server::user_agent_endpoint(const i32 client_fd,const hashMap<string,string>& headers,bool should_close){

   if(headers.find("User-Agent")==headers.end()){
       return not_found(client_fd);
   }
   
   string res=response(STATUS::OK,headers.at("User-Agent"),should_close);
   return send_all(client_fd,res.c_str(),res.size());
}





ssize_t Server::echo_endpoint(const string& path,const i32 client_fd,bool should_close){
   string res=response(STATUS::OK,extract_request_body_from_path(path),should_close);
   
   return send_all(client_fd,res.c_str(),res.size());
}


ssize_t Server::post_file_endpoint(const string& path,const i32 client_fd,string& body,bool should_close){
       
           write_response_to_file(path,body);
           string res=response(STATUS::OK,"Created successfully",should_close);
           return send_all(client_fd,res.c_str(),res.size());
        
}



ssize_t Server::get_file_endpoint(const i32 client_fd,const string& path,bool should_close){
   

      size_t file_size;
      i8 *buffer=read_file_contents(path,file_size);
       
      std::string body(buffer,file_size);
      string res= response(STATUS::OK,body,should_close);
      delete[] buffer;
      return send_all(client_fd,res.c_str(),res.size());


}


ssize_t Server::put_file_endpoint(const string& path,const i32 client_fd,string& body,bool should_close){
    std::filesystem::path file_path(path);
    
    bool file_exists=std::filesystem::exists(file_path);

    std::filesystem::path parent_directory=file_path.parent_path();

    if(!std::filesystem::exists(parent_directory))
            std::filesystem::create_directories(parent_directory);

    std::ofstream file(file_path,std::ios::binary | std::ios::trunc);

    if(!file.is_open())
         throw FileIOException("Failed to open file: "+path);

    file.write(body.c_str(),body.size());
    file.close();

    string res;
    if(file_exists){
       res=response(STATUS::OK,"File replaced successfully",should_close);
    }else{
       res=response(STATUS::CREATED,"File created successully",should_close);
    }

    return send_all(client_fd,res.c_str(),res.size());
   
}

ssize_t Server::patch_file_endpoint(string& body,const string& path,const i32 client_fd,bool should_close){
      std::filesystem::path file_path(path);

      if(!std::filesystem::exists(file_path)){
           throw FileIOException("File not found: "+path);
      }

      std::ofstream file(file_path,std::ios::binary | std::ios::app);

      if(!file.is_open()){
          throw FileIOException("Failed to open file: "+path);
      }

      file<<body;

      file.close();

      string res=response(STATUS::OK,"File updated successfully",should_close);
      return send_all(client_fd,res.c_str(),res.size());
}

ssize_t Server::delete_file_endpoint(const i32 client_fd,const string& path,bool should_close){
      std::filesystem::path file_path(path);
      string res;
      if(std::filesystem::remove(file_path)){
           res=response(STATUS::OK,"Deleted successfully",should_close);
           return send_all(client_fd,res.c_str(),res.size());
      }else{
           res=response(STATUS::NOT_FOUND,"Not Found",should_close);
           return send_all(client_fd,res.c_str(),res.size());
      }

      res=response(STATUS::INTERNAL_SERVER_ERROR,"Internal Server Error",should_close);
      return send_all(client_fd,res.c_str(),res.size());

}

//read and write to a file in the server
 void  Server::write_response_to_file(const string &path,string& body){
     std::filesystem::path f_path(path);
     
     std::filesystem::path parent_dir=f_path.parent_path();

     if(!std::filesystem::exists(parent_dir)){
       
      std::filesystem::create_directories(parent_dir);
     }

   

     if(!parent_dir.has_filename()){
        throw FileIOException("Invalid path from the client: "+path);
     }

     std::ofstream file(f_path,std::ios::binary);
     
     if(!file.is_open()){
        throw FileIOException("Failed to open file: "+path);
     }

     file.write(body.c_str(),body.size());
     file.close();
      
}


i8* Server::read_file_contents(const string& path,size_t& file_size){
   
   std::ifstream file(path,std::ios::in | std::ios::binary | std::ios::ate);
   
    if(!file.is_open()){
      throw FileIOException("Failed to open file : "+path);
    }

    file_size=file.tellg();
    file.seekg(0);

    i8 *buffer=new i8[file_size];

    file.read(buffer,file_size);

    if(!file){
       delete[] buffer;
       throw FileIOException("Failed to read the file: "+path);
    }

    return buffer;

}

/* 
   Methods to:
   Extracting the request line,
   Extract request headers,
   Extract request body
*/


vector<string> Server::extract_request_line(const i8 *buffer){
   string request(buffer);
   std::istringstream raw_request_line(request);
   string request_line;
   
   std::getline(raw_request_line,request_line);

   if(!request_line.empty() && request_line.back()=='\r'){
       request_line.pop_back();
   }
   std::istringstream line_stream(request_line);
   string method,path,version;
   line_stream >> method >> path >> version;

   return {method,path,version};
   
}


hashMap<string,string> Server::extract_headers(const i8 *buffer){
       
      hashMap<string,string> headers;
       
       std::istringstream buffer_stream(buffer);
       string line;

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

         if(colon_position!=string::npos){
          
            string key=line.substr(0,colon_position);
            string value=line.substr(colon_position+1);

            /*
               Header is usually something like Host: localhost:port \r\n
               right now we have trimmed the crlf

               now we have the Host as the key ,and the localhost and port and the value ,but the value might
               contain some spaces at the start and we shoulheaders[key]=value;d get rid of that

            */

            size_t first_non_space_position=value.find_first_not_of(" \t");
            if(first_non_space_position!=string::npos){
                value=value.substr(first_non_space_position);
            }else{
               value.clear();
            }

            headers[key]=value;

         }

       }

       return headers;
    
}



/*
   Server starts from this point
*/


void Server::start_server(i8 *__directory){
   i32 server_fd=socket(AF_INET,SOCK_STREAM,0);
    

  
    make_socket_non_blocking(server_fd);

   
   if(server_fd<0){
      error("socket FD creation error");
      exit(EXIT_FAILURE);
   }
   
   i32 reuse=1;
   
   if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0){
      error("Setting the address to be reusable failed");
      exit(EXIT_FAILURE);
   }
   
   SA server_address;
   
   server_address.sin_family=AF_INET;
   server_address.sin_addr.s_addr=INADDR_ANY;
   server_address.sin_port=htons(PORT);
   
   if(bind(server_fd,(const struct sockaddr *)&server_address,sizeof(server_address))!=0){
      error("Failed to bind the server file descriptor to this address");
      exit(EXIT_FAILURE);
   }
   
   i32 connection_backlog=50;
   if(listen(server_fd,connection_backlog)!=0){
      error("Failed to listen on PORT 4221");
      exit(EXIT_FAILURE);
   }
   
   
  
   
   std::cout<< WHITE << "Waiting for client connection" <<RESET <<std::endl;
     
    


     /*
        We will monitor this socket for incoming client connections
     */

     epoll_event event{};
     event.data.fd=server_fd;
     event.events=EPOLLIN;

     if(epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event)==-1){
        error("Failed to add the server fd to epoll");
        exit(EXIT_FAILURE);
     }

     std::vector<epoll_event> events(1024);
 
         
    FDGuard guard(server_fd,epfd, mtx);
  


   
   while(1){
      
        i32 n;
       {
          std::unique_lock<std::mutex> lock(mtx);
          n=epoll_wait(epfd,events.data(),events.size(),-1);
       }
      
      if(n==-1){
          error("Poll failed");
          continue;
      }
  
     
   
    

      for(int i=0;i<n;i++){
            
              if(events[i].data.fd==server_fd){
                    i32 client_fd=accept_client_connection(server_fd);
   
                  if(client_fd>=0){
                       make_socket_non_blocking(client_fd);
                       
                       epoll_event client_event{};

                       client_event.data.fd=client_fd;
                       client_event.events=EPOLLIN;
                       {
                          std::unique_lock<std::mutex> lock(mtx);
                          epoll_ctl(epfd,EPOLL_CTL_ADD,client_fd,&client_event);
                       }
                       
                     
                  }
   
        
              }else{

                  
                  
                   CLIENT_ARGS client_args;
             
                   client_args.client_fd=events[i].data.fd;
                   client_args.file_path=__directory;

                   {
                               std::unique_lock<std::mutex> lock(mtx);
                             epoll_ctl(epfd,EPOLL_CTL_DEL,client_args.client_fd,nullptr);
                   }
                
                  thread_pool.enqueue([this, client_fd = client_args.client_fd, directory = client_args.file_path](){
                     handle_client({client_fd, directory});
                       });

                  }


         

      }

        
   }


     


}

/*
   Helpers of start server function
*/


i32 Server::make_socket_non_blocking(i32 fd){
        i32 flags=fcntl(fd,F_GETFL,0);
        if(flags==-1) return -1;
       return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}


i32 Server::accept_client_connection(i32 server_fd){

                   SA client_address;
                   socklen_t client_address_size=sizeof(client_address);
                  i32 client_fd=accept(server_fd,(struct sockaddr *)&client_address,&client_address_size);
                  if(client_fd==-1){

                      if (errno != EAGAIN && errno != EWOULDBLOCK) {
                          error("Failed to accept connection");
                      }

                    return -1;
                }
                  


                  return client_fd;
    
}

/*
   All client requests will be handled here
*/



void Server::handle_client(const CLIENT_ARGS& client_args){

   
    try{


      while(true){

         string headers=read_headers(client_args.client_fd);

         if(headers.empty()) break;
       
         string content_length_string=get_header_value(headers,HEADERS::CONTENT_LEN);

         string connection_header=get_header_value(headers,HEADERS::CONNECTION);

         bool should_close=(connection_header=="close");

         // bool should_close=true;
         
         size_t content_len=content_length_string.empty()?0:std::stoul(content_length_string);
        
         string body=read_body(client_args.client_fd,headers,content_len);
           
        
         
         vector<string> request_line=extract_request_line(reinterpret_cast<const i8*>(headers.c_str()));
           
         /*
           The above vector contains :
           Request method at the first index:[0]
           Request path at the second index:[1]
           HTTP version at the third index:[2]
   
          */
   
          
   
         string path=request_line[1];
         string METHOD=request_line[0];
   
         ssize_t bytes_sent;
      
         if(path.starts_with("/echo/")){
            
            bytes_sent=echo_endpoint(path,client_args.client_fd,should_close);
         }else if(path.starts_with("/user-agent")){
             hashMap<string,string> headers__=extract_headers(reinterpret_cast<const i8*>(headers.c_str()));
             bytes_sent=user_agent_endpoint(client_args.client_fd,headers__,should_close);
         }else if(path.starts_with("/files/") && client_args.file_path){
              
                 string directory(client_args.file_path);
                 string file_name=path.substr(strlen("/files/"));
                 string full_path=directory+file_name;
                if(METHOD=="GET"){
                   bytes_sent=get_file_endpoint(client_args.client_fd,full_path,should_close);
                }else if(METHOD=="POST"){
                    
                    bytes_sent=post_file_endpoint(full_path,client_args.client_fd,body,should_close);
                }else if(METHOD=="DELETE"){
                    bytes_sent=delete_file_endpoint(client_args.client_fd,full_path,should_close);
                }else if(METHOD=="PUT"){
                     bytes_sent=put_file_endpoint(full_path,client_args.client_fd,body,should_close);
                }else if(METHOD=="PATCH"){
                     bytes_sent=patch_file_endpoint(body,full_path,client_args.client_fd,should_close);
                }
         }
   
          
         if(bytes_sent==-1){ 
            throw ServerException("Error sending response");
         }

         if(should_close) break;
      

             epoll_event ev{};
            ev.data.fd = client_args.client_fd;
            ev.events = EPOLLIN; 
            std::unique_lock<std::mutex> lock(mtx);
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_args.client_fd, &ev) == -1) {
              
               if (errno == EEXIST)
                     epoll_ctl(epfd, EPOLL_CTL_MOD, client_args.client_fd, &ev);
            }

            return;
      }

       
 
    }catch(const ServerException& e){
         
        std::cerr<<RED<<e.what()<<RESET<<"\n";
        std::cout<<strerror(errno)<<" "<<client_args.client_fd<<std::endl;
        internal_server_error(client_args.client_fd);

    }catch(const NetworkException& e){
        std::cerr<<RED<<e.what()<<RESET<<"\n";
        internal_server_error(client_args.client_fd);

   }catch(const FileIOException& e){
         string __error=e.what();
         std::cerr<<RED<<__error<<RESET<<"\n";
       
         if(__error.find("Invalid path")!=std::string::npos || __error.find("not found")!=std::string::npos){
                      not_found(client_args.client_fd);
         }else{
              internal_server_error(client_args.client_fd);
         }
        



   }catch(...){
        std::cerr<<RED<<"Unhandled exception"<<RESET<<"\n";
        internal_server_error(client_args.client_fd);
      
      
    }

   close(client_args.client_fd);

}


string Server::read_body(i32 client_fd,string& headers,size_t content_len){

     
    i8 buffer[BUFFER_SIZE]={0};
    string body;

    size_t headers_end=headers.find("\r\n\r\n");
    if(headers_end!=string::npos){
       size_t body_start=headers_end+4;

       if(headers.size()>body_start){
          body=headers.substr(body_start);
       }

    }

    

   while(body.size()<content_len){

         //  FDGuard guard(client_fd,epfd, mtx);
 
          ssize_t received_bytes=recv(client_fd,buffer,BUFFER_SIZE-1,0);
            
          if(received_bytes>0){
               
               body.append(buffer,received_bytes);
               continue;
             }else if(received_bytes==0){
                
                break;
            }else if(received_bytes<0){
                     if(errno==EAGAIN || errno==EWOULDBLOCK){
                      break;
                   }else if (errno == ECONNRESET || errno == EBADF || errno == ENOTCONN) {
                         break;
                } else{
                   
                      throw NetworkException("Error receiving client request");
                   }
                
          }

          
       }

   
   if(body.size() > content_len){
        body.resize(content_len);
    }



    return body;
}

//This will read the request until the end of the headers.

string Server::read_headers(i32 client_fd){

      i8 buffer[BUFFER_SIZE]={0};
       string request_data;
     
       while(true){

        
 
          ssize_t received_bytes=recv(client_fd,buffer,BUFFER_SIZE-1,0);
            
          if(received_bytes>0){
               
               request_data.append(buffer,received_bytes);
               if(request_data.find("\r\n\r\n")!=string::npos){
                     break;
               }
            
               continue;
          }else if(received_bytes==0){
                  {  
                    std::unique_lock<std::mutex> lock(mtx);
                    std::cout<<GREEN<<"Client disconnected\n"<<RESET;
                  }
                break;
          }else if(received_bytes<0){
                   if(errno==EAGAIN || errno==EWOULDBLOCK){
                     
                      break;
                   }else if (errno == ECONNRESET || errno == EBADF || errno == ENOTCONN) {
          
                         break;
                } else{
                   
                      throw NetworkException("Error receiving client request");
                   }
                
          }

          
       }

       return request_data;

}


/*
  send helper
*/


ssize_t Server::send_all(i32 client_fd, const i8* data, size_t len) {
   
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = send(client_fd, data + total_sent, len - total_sent, 0);
        if (sent > 0) {
            total_sent += sent;
        } else if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                return -1; 
            }
        }
    }
    return total_sent;
}






/*
  Responses 
*/

ssize_t Server::internal_server_error(const i32 client_fd){
             string res=response(STATUS::INTERNAL_SERVER_ERROR,"Internal Server Error",true);
             return send_all(client_fd,res.c_str(),res.size());
}


 ssize_t Server::not_found(const i32 client_fd){
             string res=response(STATUS::NOT_FOUND,"Not Found",true);
             return send_all(client_fd,res.c_str(),res.size());
 }


string Server::response(const STATUS status,const string& __body,bool should_close){
      
      return std::format(
           "HTTP/1.1 {}\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: {}\r\n"
           "Connection: {}\r\n"
           "\r\n"
           "{}",
           status_code_to_string(status),
           __body.size(),
           should_close?"close":"keep-alive",
           __body   
      );
}

