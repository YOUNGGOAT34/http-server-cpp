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



class ThreadPool{


//private members
   private:
         std::vector<std::thread> worker_threads;
         std::mutex mtx;
         std::condition_variable cv;
         std::queue<std::function<void()>> tasks;
         std::atomic<bool> stopping=false;
         

//public methods
   public:

         ThreadPool(size_t number_of_threads);
         ~ThreadPool();
         template<typename F,typename ...Args>
          void enqueue(F&& f,Args&& ...args){
                    auto func=[f=std::forward<F>(f),...args=std::forward<Args>(args)](){
                               f(args...);
                             };

                        {
                              std::unique_lock<std::mutex> lock(mtx);
                              tasks.push(func);
                        }
      
                           cv.notify_one();
          }
//private methods
   private:
         void start_threads(size_t number_of_threads);
         void stop();
         
};









#endif