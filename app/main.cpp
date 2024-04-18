#include<iostream>

#include "YThreadPool/ThreadPool.h"
#include "YThreadPool/Queue/LockFreeRingBufferQueue.h"
using namespace YThreadPool;

int add(int i, int j) {
	return i + j;
}
class MyFunction {
public:
    int operator() (int i, int j) {
        return i + j;
    }

    static int divide(int i, int j) {
        if (0 == j) {
            return 0;
        }

        return i / j;
    }
private:
    int power_ = 2;
};


void threadpool_1(ThreadPoolPtrs tp) {
    /**
     * 依次向线程池中传入：
     * 1、普通函数
     * 2、静态函数
     * 3、类成员函数
     * 4、仿函数
     */
    int i = 2, j = 1;
    auto r1 = tp->commit([i, j] {return add(i, j); });
    std::future<int> r2 = tp->commit(std::bind(add, i, j));
    auto r3 = tp->commit(std::bind(&MyFunction::divide, i, j));
    std::future<int> r4 = tp->commit(std::bind(MyFunction(), i, j));

    std::cout << r1.get() << std::endl;    
    std::cout << r2.get() << std::endl;    
    std::cout << r3.get() << std::endl;   
    std::cout << r4.get() << std::endl;
}


void threadpool_2(ThreadPoolPtrs tp) {
    /**
     * 通过添加工作组(taskGroup)来执行任务
     */
    TaskGroup taskGroup;

    /** 添加一个不耗时的任务 */
    int i = 1, j = 2, k = 3;
    auto func1 = [] {std::cout << "Hello,Thread" << std::endl; return; };
    taskGroup.addTask(func1);

    /** 添加一个耗时为1000ms的任务 */
    taskGroup.addTask([i, j] {
        int result = i + j;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout<<"sleep for 1 second, "<<i<<" + "<<j<<" = "<<result<<", run success."<<std::endl;
    });

    taskGroup.addTask([i, j, k] {
        int result = i - j + k;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        std::cout << "sleep for 2 second, " << i << " - " << j << " + " << k << " = " << result << ", run success." << std::endl;;
        return result;    // submit接口，不会对线程函数返回值进行判断。如果需要判断，考虑commit方式
    });

    /**
     * 设定超时时间=2500ms，确保以上任务能顺利执行完成
     * 如果加入sleep(3000)的任务，则也会在2500ms的时候退出
     * 并且在status中提示超时信息
     * */
    std::string res = tp->submit(taskGroup, 2500);
    std::cout << "res = " << res << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void threadpool_3(ThreadPoolPtrs tp) {
    /**
     * 并发打印0~100之间的数字
     * 使用commit和submit函数的区别，主要在于：
     * 1，commit()属于非阻塞执行，是将线程函数执行的结果以future的类型返回，交由上层处理
     * 2，submit()属于阻塞顺序执行，是在内部处理好超时等信息并作为结果返回，抛弃线程函数自身返回值
     * 3，不需要线程函数返回值，并且不需要判断超时信息的场景，两者无区别（如下例）
     */
    const int size = 100;
    std::cout << "thread pool task submit version : " << std::endl;
    for (int i = 0; i < size; i++) {
        tp->submit([i] { std::cout << i << " "; });    
        // 可以看到，submit版本是有序执行的。如果需要想要无序执行，可以通过创建taskGroup的方式进行，或者使用commit方法
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));// 等待上面函数执行完毕，以便于观察结果。无实际意义
    std::cout << std::endl;

    std::cout << "thread pool task group submit version : " << std::endl;
    TaskGroup taskGroup;
    for (int i = 0; i < size; i++) {
        taskGroup.addTask([i] { std::cout << i << " "; });    
        // 将任务放到一个taskGroup中，并发执行。执行的结果是无序的
    }
    tp->submit(taskGroup);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::endl;

    std::cout << "thread pool task commit version : " << std::endl;
    for (int i = 0; i < size; i++) {
        tp->commit([i] { std::cout << i << " "; });    
        // commit版本，是无序执行的
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::endl;
}


int main() {

	ThreadPoolPtrs YThread_Pool = std::make_shared<ThreadPool>(true);
    
    std::cout << "======== threadpool_1 begin. ========" << std::endl;
    threadpool_1(YThread_Pool);
    std::cout << "======== threadpool_2 begin. ========" << std::endl;
    threadpool_2(YThread_Pool);
    std::cout << "======== threadpool_3 begin. ========" << std::endl;
    threadpool_3(YThread_Pool);

	return 0;
}
