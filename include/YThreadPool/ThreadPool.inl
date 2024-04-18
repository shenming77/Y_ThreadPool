

#ifndef THREADPOOL_INL
#define THREADPOOL_INL

#include "YThreadPool/ThreadPool.h"

namespace YThreadPool{

template<typename FunctionType>
auto ThreadPool::commit(const FunctionType& task, int index)
-> std::future<decltype(std::declval<FunctionType>()())> {
    using ResultType = decltype(std::declval<FunctionType>()());//�ڱ���ʱ����������������(���캯��)�����������Ƶ�

    std::packaged_task<ResultType()> runnableTask(std::move(task));//ģ����,��װ��һ���첽����
    std::future<ResultType> result(runnableTask.get_future()); //�첽�����д��벢��δ��ĳ��ʱ�̻�ȡ�������

    int realIndex = dispatch(index);
    if (realIndex >= 0 && realIndex < config_.default_thread_size_) {
        // ������صĽ���������߳�����֮�䣬��ŵ����̵߳�queue��ִ��
        primary_threads_[realIndex]->pushTask(std::move(runnableTask));
    }
    else if (LONG_TIME_TASK_STRATEGY == realIndex) {
        /**
         * ����ǳ�ʱ�������򽻸��ض���������У����ɸ����̴߳���
         * Ŀ���Ƿ�ֹ�кܶ೤ʱ�����񣬽��������е��߳̾�����
         * ���������Ĭ�����ȼ��ϵ�
         **/
        priority_task_queue_.push(std::move(runnableTask), LONG_TIME_TASK_STRATEGY);
    }
    else {
        // ��������������ŵ�pool��queue��ִ��
        task_queue_.push(std::move(runnableTask));
    }
    return result;
}


template<typename FunctionType>
auto ThreadPool::commitWithTid(const FunctionType& task, int tid, bool enable, bool lockable)
-> std::future<decltype(std::declval<FunctionType>()())> {
    using ResultType = decltype(std::declval<FunctionType>()());
    std::packaged_task<ResultType()> runnableTask(std::move(task));
    std::future<ResultType> result(runnableTask.get_future());

    if (tid >= 0 && tid < config_.default_thread_size_) {
        primary_threads_[tid]->pushTask(std::move(runnableTask), enable, lockable);
    }
    else {
        // ����������̵߳ķ�Χ����Ĭ��д�� pool ͨ�õ����������
        task_queue_.push(task);
    }
    return result;
}


template<typename FunctionType>
auto ThreadPool::commitWithPriority(const FunctionType& task, int priority)
-> std::future<decltype(std::declval<FunctionType>()())> {
    using ResultType = decltype(std::declval<FunctionType>()());

    std::packaged_task<ResultType()> runnableTask(task);
    std::future<ResultType> result(runnableTask.get_future());

    if (secondary_threads_.empty()) {
        createSecondaryThread(1);    // ���û�п��������̣߳���ֱ�ӿ���һ��
    }

    priority_task_queue_.push(std::move(runnableTask), priority);
    return result;
}

}
#endif   
