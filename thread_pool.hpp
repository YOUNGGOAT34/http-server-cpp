#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <functional>
#include <queue>
#include <atomic>


template<typename T>
class ThreadPool{


//private members
   private:
         std::vector<std::thread> worker_threads;
         std::mutex mtx;
         std::condition_variable cv;
         std::queue<T> tasks;
         std::atomic<bool> stopping=false;
         std::function<void(T)> task_handler;


//public methods
   public:
         using Task=std::function<void(T)>;

         ThreadPool(size_t number_of_threads);
         ~ThreadPool();
          void set_task_handler(Task handler);
           void enqueue(T arg);
//private methods
   private:
         void start_threads(size_t number_of_threads);
         void stop();



        
         
};


template<typename T>
ThreadPool<T>::ThreadPool(size_t number_of_threads){
        start_threads(number_of_threads);
}


template<typename T>
ThreadPool<T>::~ThreadPool(){
     stop();
}


template<typename T>
void ThreadPool<T>::start_threads(size_t number_of_threads){
     for(size_t i=0;i<number_of_threads;++i){
              worker_threads.emplace_back([this]{
              while(true){
                 T arg;

                 {
                       std::unique_lock<std::mutex> lock(mtx);

                       cv.wait(lock,[this]{
                        return stopping || !tasks.empty();
                     });

                     if(stopping && tasks.empty()){
                         return;
                     }

                     arg=tasks.front();
                     tasks.pop();



                 }

                 if(task_handler){
                    task_handler(arg);
                 }
              }

        });
     }
}

template<typename T>
 void ThreadPool<T>::set_task_handler(Task handler){
            task_handler=handler;
 }

 template<typename T>
 void ThreadPool<T>::enqueue(T arg){
      {
         std::unique_lock<std::mutex> lock(mtx);
         tasks.push(arg);
      }

      cv.notify_one();
 }


 template<typename T>
void ThreadPool<T>::stop(){
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






#endif