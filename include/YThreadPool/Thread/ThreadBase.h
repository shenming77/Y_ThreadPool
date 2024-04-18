

#ifndef THREADBASE_H
#define THREADBASE_H

#include <thread>
#include <cassert>

#include "YThreadPool/Queue/AtomicQueue.h"
#include "YThreadPool/Queue/WorkStealingQueue.h"
#include "YThreadPool/Queue/WorkStealingLockFreeQueue.h"
#include "YThreadPool/Queue/AtomicPriorityQueue.h"
#include "YThreadPool/Queue/AtomicRingBufferQueue.h"
#include "YThreadPool/Queue/LockFreeRingBufferQueue.h"
#include "YThreadPool/Task/Task.h"
#include "YThreadPool/Task/TaskGroup.h"
#include "YThreadPool/ThreadPoolConfig.h"

#include "YThreadPool/Metrics.h"


namespace YThreadPool{

class ThreadBase {
protected:
    explicit ThreadBase() {
        done_ = true;
        is_init_ = false;
        is_running_ = false;
        pool_task_queue_ = nullptr;
        pool_priority_task_queue_ = nullptr;
        config_ = nullptr;
        total_task_num_ = 0;
    }


    virtual ~ThreadBase() {
        reset();
    }

    virtual std::string init() {
        return std::string{};
    }

    virtual std::string run()  {
        return std::string{"function no support"};
    }

    virtual std::string destroy(){
        std::string res;
        
        reset();
        return res;
    }


    /**
     * ���̳߳صĶ����У���ȡ����
     * @param task
     * @return
     */
    virtual bool popPoolTask(TaskRef task) {
        bool result = pool_task_queue_->tryPop(task);
        if (!result && THREAD_TYPE_SECONDARY == type_) {
            // ��������߳�û�л�ȡ���Ļ�������Ҫ�ٳ��Դӳ�ʱ����������У���ȡһ��
            result = pool_priority_task_queue_->tryPop(task);
        }
        metrics_.calcPool(result, 1);
        return result;
    }


    /**
     * ���̳߳صĶ������У���ȡ��������
     * @param tasks
     * @return
     */
    virtual bool popPoolTask(TaskArrRef tasks) {
        bool result = pool_task_queue_->tryPop(tasks, config_->max_pool_batch_size_);
        if (!result && THREAD_TYPE_SECONDARY == type_) {
            result = pool_priority_task_queue_->tryPop(tasks, 1);    // �����ȶ�������pop����һ��
        }

        metrics_.calcPool(result, tasks.size());
        return result;
    }


    /**
     * ִ�е�������
     * @param task
     */
    void runTask(Task& task) {
        is_running_ = true;
        
        try{
            task();
        } catch (const char* msg) {
            std::cerr << "Caught an exception: " << msg << std::endl;
        }

        total_task_num_++;
        is_running_ = false;
    }


    /**
     * ����ִ������
     * @param tasks
     */
    void runTasks(TaskArr& tasks) {
        is_running_ = true;
        for (auto& task : tasks) {
            task();
        }
        total_task_num_ += tasks.size();
        is_running_ = false;
    }


    /**
     * ���������������
     */
    void reset() {
        done_ = false;
        cv_.notify_one();    // ��ֹ���߳� waitʱ����������µĽ�����������
        if (thread_.joinable()) {
            thread_.join();    // �ȴ��߳̽���
        }
        is_init_ = false;
        is_running_ = false;
        total_task_num_ = 0;
    }

    /**
     * ִ�е�����Ϣ
     * @return
     */
    virtual void processTask() = 0;


    /**
     * ��ȡ����ִ��task��Ϣ
     */
    virtual void processTasks() = 0;


    /**
     * ѭ����������
     * @return
     */
    std::string loopProcess() {
        std::string res;

        assert(config_ != nullptr);

       if (config_->batch_task_enable_) {
            while (done_) {
                processTasks();    // ���������ȡִ�нӿ�
            }
       }
       else {
            while (done_) {
                processTask();    // ���������ȡִ�нӿ�
            }
       }

       return res;
    }


    /**
    * �����߳����ȼ�������Է�windowsƽ̨ʹ��
    */
    void setSchedParam() {
#ifndef _WIN32
        int priority = THREAD_SCHED_OTHER;
        int policy = THREAD_MIN_PRIORITY;
        if (type_ == THREAD_TYPE_PRIMARY) {
            priority = config_->primary_thread_priority_;
            policy = config_->primary_thread_policy_;
        }
        else if (type_ == THREAD_TYPE_SECONDARY) {
            priority = config_->secondary_thread_priority_;
            policy = config_->secondary_thread_policy_;
        }

        auto handle = thread_.native_handle();
        sched_param param = { calcPriority(priority) };
        int ret = pthread_setschedparam(handle, calcPolicy(policy), &param);
        if (0 != ret) {
            printf("warning : set thread sched param failed, system error code is [%d]", ret);
        }
#endif
    }

    /**
     * �����߳��׺��ԣ������linuxϵͳ
     */
    void setAffinity(int index) {
#if defined(__linux__) && !defined(__ANDROID__)
        if (!config_->bind_cpu_enable_ || CPU_NUM == 0 || index < 0) {
            return;
        }

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(index % CPU_NUM, &mask);

        auto handle = thread_.native_handle();
        int ret = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &mask);
        if (0 != ret) {
            printf("warning : set thread affinity failed, system error code is [%d]", ret);
        }
#endif
    }


private:
    /**
     * �趨�����̵߳��Ȳ�����Ϣ��
     * ��OTHER/RR/FIFO��Ӧ��ֵ��ͳһ����OTHER����
     * @param policy
     * @return
     */
    static int calcPolicy(int policy) {
        return (THREAD_SCHED_OTHER == policy
            || THREAD_SCHED_RR == policy
            || THREAD_SCHED_FIFO == policy)
            ? policy : THREAD_SCHED_OTHER;
    }


    /**
     * �趨�߳����ȼ���Ϣ
     * ����[min,max]��Χ��ͳһ����Ϊminֵ
     * @param priority
     * @return
     */
    static int calcPriority(int priority) {
        return (priority >= THREAD_MIN_PRIORITY
            && priority <= THREAD_MAX_PRIORITY)
            ? priority : THREAD_MIN_PRIORITY;
    }


protected:
    bool done_;                                                        // �߳�״̬���
    bool is_init_;                                                     // ��ǳ�ʼ��״̬
    bool is_running_;                                                  // �Ƿ�����ִ��
    int type_ = 0;                                                     // ���������߳����ͣ����̡߳������̣߳�
    unsigned long total_task_num_ = 0;                                 // ��������������

    AtomicQueue<Task>* pool_task_queue_;                             // ���ڴ���̳߳��е���ͨ����
    AtomicPriorityQueue<Task>* pool_priority_task_queue_;            // ���ڴ���̳߳��еİ������ȼ�����Ķ��У��������߳̿���ִ��
    ThreadPoolConfigPtr config_ = nullptr;                            // ���ò�����Ϣ
    Metrics metrics_;                                                 // �߳��е�ָ����Ϣ

    std::thread thread_;                                               // �߳���
    std::mutex mutex_;
    std::condition_variable cv_;
};

}

#endif 
