#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include "YThreadPool/ThreadPool.h"
#include "ThreadPool/threadpool.h"

class ThreadPoolTest : public testing::Test {
protected:
    static void SetUpTestCase() {
        YThread_Pool.reset(new YThreadPool::ThreadPool(true));
        threadpool.reset(new ThreadPool(8));
    }

    static void TearDownTestCase() {
        YThread_Pool.reset();
        threadpool.reset();
    }

    static YThreadPool::ThreadPoolPtr YThread_Pool;
    static std::unique_ptr<ThreadPool> threadpool;
};

YThreadPool::ThreadPoolPtr ThreadPoolTest::YThread_Pool;
std::unique_ptr<ThreadPool> ThreadPoolTest::threadpool;

TEST_F(ThreadPoolTest, ThreadPool_test_time1) {
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


    ASSERT_TRUE(true);
}

TEST_F(ThreadPoolTest, YThreadPool_test_time1) {
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

    ASSERT_TRUE(true);
}

TEST_F(ThreadPoolTest, ThreadPool_test_time2) {
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


    ASSERT_TRUE(true);
}

TEST_F(ThreadPoolTest, YThreadPool_test_time2) {
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

    ASSERT_TRUE(true);
}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}