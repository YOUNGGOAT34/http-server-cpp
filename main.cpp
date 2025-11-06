
#include "server.hpp"
#include <iostream>



int main(i32 argc,i8 *argv[]){
   
   std::cout << std::unitbuf; 
   std::cerr << std::unitbuf;

   i8 *directory=nullptr;

 

   if(argc>1){
       for(int i=0;i<argc;i++){
           if(strcmp(argv[i],"--directory")==0){
              if(i+1<argc){

                 directory=argv[i+1];
              }else{
                 error("Expected a directory name");
              }
           }
       }
   }


   Server server;
   server.start_server(directory);


   return 0;
}

