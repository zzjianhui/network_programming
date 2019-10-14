//线程池
//获取描述符放入到队列中，等待的线程从队列中取出描述符，执行与客户端收发函数

#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<functional>
#include<iostream>

const int thread_count=8;

class ThreadPool{
public:
    ThreadPool(std::function<void(int fd)> F){
        for(int i=0;i<thread_count;++i){
            pools.push_back(std::thread(&ThreadPool::thread_run,this,std::ref(F)));
        }
    }

    ~ThreadPool(){
        {
            std::lock_guard<std::mutex> lg(m);
            is_shutdown=true;
        }
        cv.notify_all();
        for(auto it=pools.begin();it!=pools.end();it++){
            (*it).join();
        }
    }

    void thread_run(std::function<void(int fd)> F){
        std::unique_lock<std::mutex> lk(m);
        for(;;){
            if(!fd_queue.empty()){
                auto fd=fd_queue.front();
                fd_queue.pop();
                lk.unlock();
                //根据得到的描述符，运行在这个线程上运行对应的函数内容
                F(fd);
                lk.lock();
            }else if(is_shutdown){
                break;
            }else{
                cv.wait(lk);
            }
        }
    }

    void execute(int fd){
        {
            std::lock_guard<std::mutex> lg(m);
            fd_queue.push(fd);
        }
        cv.notify_one();
    }

private:
    ThreadPool(const ThreadPool&);//禁止拷贝复制
    const ThreadPool& operator=(const ThreadPool&);

    std::mutex m;
    std::condition_variable cv;
    std::queue<int> fd_queue;
    std::vector<std::thread> pools;
    bool is_shutdown=false;
};