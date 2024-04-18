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
     * �������̳߳��д��룺
     * 1����ͨ����
     * 2����̬����
     * 3�����Ա����
     * 4���º���
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
     * ͨ����ӹ�����(taskGroup)��ִ������
     */
    TaskGroup taskGroup;

    /** ���һ������ʱ������ */
    int i = 1, j = 2, k = 3;
    auto func1 = [] {std::cout << "Hello,Thread" << std::endl; return; };
    taskGroup.addTask(func1);

    /** ���һ����ʱΪ1000ms������ */
    taskGroup.addTask([i, j] {
        int result = i + j;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout<<"sleep for 1 second, "<<i<<" + "<<j<<" = "<<result<<", run success."<<std::endl;
    });

    taskGroup.addTask([i, j, k] {
        int result = i - j + k;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        std::cout << "sleep for 2 second, " << i << " - " << j << " + " << k << " = " << result << ", run success." << std::endl;;
        return result;    // submit�ӿڣ�������̺߳�������ֵ�����жϡ������Ҫ�жϣ�����commit��ʽ
    });

    /**
     * �趨��ʱʱ��=2500ms��ȷ������������˳��ִ�����
     * �������sleep(3000)��������Ҳ����2500ms��ʱ���˳�
     * ������status����ʾ��ʱ��Ϣ
     * */
    std::string res = tp->submit(taskGroup, 2500);
    std::cout << "res = " << res << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void threadpool_3(ThreadPoolPtrs tp) {
    /**
     * ������ӡ0~100֮�������
     * ʹ��commit��submit������������Ҫ���ڣ�
     * 1��commit()���ڷ�����ִ�У��ǽ��̺߳���ִ�еĽ����future�����ͷ��أ������ϲ㴦��
     * 2��submit()��������˳��ִ�У������ڲ�����ó�ʱ����Ϣ����Ϊ������أ������̺߳���������ֵ
     * 3������Ҫ�̺߳�������ֵ�����Ҳ���Ҫ�жϳ�ʱ��Ϣ�ĳ�����������������������
     */
    const int size = 100;
    std::cout << "thread pool task submit version : " << std::endl;
    for (int i = 0; i < size; i++) {
        tp->submit([i] { std::cout << i << " "; });    
        // ���Կ�����submit�汾������ִ�еġ������Ҫ��Ҫ����ִ�У�����ͨ������taskGroup�ķ�ʽ���У�����ʹ��commit����
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));// �ȴ����溯��ִ����ϣ��Ա��ڹ۲�������ʵ������
    std::cout << std::endl;

    std::cout << "thread pool task group submit version : " << std::endl;
    TaskGroup taskGroup;
    for (int i = 0; i < size; i++) {
        taskGroup.addTask([i] { std::cout << i << " "; });    
        // ������ŵ�һ��taskGroup�У�����ִ�С�ִ�еĽ���������
    }
    tp->submit(taskGroup);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::endl;

    std::cout << "thread pool task commit version : " << std::endl;
    for (int i = 0; i < size; i++) {
        tp->commit([i] { std::cout << i << " "; });    
        // commit�汾��������ִ�е�
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
