
#include <chrono>
#include <vector>
#include "YThreadPool/ThreadPool.h"
#include "ThreadPool/threadpool.h"


void ThreadPool_test_time1(std::shared_ptr<ThreadPool> threadpool) {
    std::vector<int> nums;
    int n{10000};
    std::mutex mx;
    for(int i=0;i<n;++i){
        threadpool->AddTask([&,i=i](){
            for(int k=i;k>=0;--k);
            std::lock_guard<std::mutex> locker(mx);
            nums.emplace_back(i);
        });
    }

    while(nums.size()!=n)std::this_thread::sleep_for(std::chrono::nanoseconds(50000));//0.05 毫秒

}

void YThreadPool_test_time1(YThreadPool::ThreadPoolPtrs YThread_Pool) {
    std::vector<int> nums;
    int n{10000};
    std::mutex mx;
    for(int i=0;i<n;++i){
        YThread_Pool->commit([&,i=i](){
            for(int k=i;k>=0;--k);
            std::lock_guard<std::mutex> locker(mx);
            nums.emplace_back(i);
        });
    }

    while(nums.size()!=n)std::this_thread::sleep_for(std::chrono::nanoseconds(50000));//0.05 毫秒

}

void ThreadPool_test_time2(std::shared_ptr<ThreadPool> threadpool) {
    std::vector<int> nums;
    int n{1000};
    std::mutex mx;
    for(int i=0;i<n;++i){
        threadpool->AddTask([&,i=i](){
            std::this_thread::sleep_for(std::chrono::nanoseconds(100000)); // 模拟耗时任务
            std::lock_guard<std::mutex> locker(mx);
            nums.emplace_back(i);
        });
    }

    while(nums.size()!=n)std::this_thread::sleep_for(std::chrono::nanoseconds(50000));//0.05 毫秒

}

void YThreadPool_test_time2(YThreadPool::ThreadPoolPtrs YThread_Pool) {
    std::vector<int> nums;
    int n{1000};
    std::mutex mx;
    for(int i=0;i<n;++i){
        YThread_Pool->commit([&,i=i](){
            std::this_thread::sleep_for(std::chrono::nanoseconds(100000)); // 模拟耗时任务
            std::lock_guard<std::mutex> locker(mx);
            nums.emplace_back(i);
        });
    }

    while(nums.size()!=n)std::this_thread::sleep_for(std::chrono::nanoseconds(50000));//0.05 毫秒

}



int main(int argc, char **argv) {
    YThreadPool::ThreadPoolPtrs YThread_Pool=std::make_shared<YThreadPool::ThreadPool>(true);
    std::shared_ptr<ThreadPool> threadpool=std::make_shared<ThreadPool>(8);
    ThreadPool_test_time1(threadpool);
    YThreadPool_test_time1(YThread_Pool);
    ThreadPool_test_time2(threadpool);
    YThreadPool_test_time2(YThread_Pool);

    return 0;
}