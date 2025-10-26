
#include "server.hpp"
#include <iostream>



int main(i32 argc,i8 *argv[]){
   
   std::cout << std::unitbuf; 
   std::cerr << std::unitbuf;

   i8 *directory=nullptr;

   if(argc>1){
       for(int i=0;i<argc;i++){
           if(strcmp(argv[i],"--directory")==0){
              directory=argv[i];
           }
       }
   }

   if(directory){
       std::cout<<"Not null\n";
   }else{
        std::cout<<"null\n";
   }

   return 0;

   Server server;
   server.start_server();

   return 0;
}

