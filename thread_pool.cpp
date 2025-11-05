#include "thread_pool.hpp"


ThreadPool::ThreadPool(size_t number_of_threads){
        start_threads(number_of_threads);
}



ThreadPool::~ThreadPool(){
     stop();
}



void ThreadPool::start_threads(size_t number_of_threads){
     for(size_t i=0;i<number_of_threads;++i){
              worker_threads.emplace_back([this]{
              while(true){
                 std::function<void()> task;

                 {
                       std::unique_lock<std::mutex> lock(mtx);

                       cv.wait(lock,[this]{
                        return stopping || !tasks.empty();
                     });

                     if(stopping && tasks.empty()){
                         return;
                     }

                     task=tasks.front();
                     tasks.pop();



                 }

                 task();



              }

        });
     }
}


void ThreadPool::stop(){
     {
       std::unique_lock<std::mutex> lock(mtx);
       stopping=true;
     }

     cv.notify_all();

     for(auto &t:worker_threads){
        if(t.joinable()){
          t.join();
        }
     }
}