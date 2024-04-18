

#ifndef UTHREADSECONDARY_H
#define UTHREADSECONDARY_H

#include "YThreadPool/Thread/ThreadBase.h"

namespace YThreadPool{

class ThreadSecondary : public ThreadBase {
public:
    explicit ThreadSecondary() {
        cur_ttl_ = 0;
        type_ = THREAD_TYPE_SECONDARY;
    }


protected:
    std::string init() override {
        std::string res;

        assert(config_ != nullptr);

        cur_ttl_ = config_->secondary_thread_ttl_;
        is_init_ = true;
        thread_ = std::move(std::thread(&ThreadSecondary::run, this));
        setSchedParam();

        return res;
    }


    /**
     * ����pool����Ϣ
     * @param poolTaskQueue
     * @param poolPriorityTaskQueue
     * @param config
     * @return
     */
    std::string setThreadPoolInfo(AtomicQueue<Task>* poolTaskQueue,
        AtomicPriorityQueue<Task>* poolPriorityTaskQueue,
        ThreadPoolConfigPtr config) {
        std::string res;

        assert(poolTaskQueue != nullptr);
        assert(poolPriorityTaskQueue != nullptr);
        assert(config != nullptr);

        this->pool_task_queue_ = poolTaskQueue;
        this->pool_priority_task_queue_ = poolPriorityTaskQueue;
        this->config_ = config;

        return res;
    }


    std::string run() final {
        std::string res;
        
        res += loopProcess();
        
        return res;
    }


    void processTask() override {
        Task task;
        if (popPoolTask(task)) {
            runTask(task);
        }
        else {
            // ��������޷���ȡ�����Լӵȴ�
            waitRunTask(config_->queue_emtpy_interval_);
        }
    }


    void processTasks() override {
        TaskArr tasks;
        if (popPoolTask(tasks)) {
            runTasks(tasks);
        }
        else {
            waitRunTask(config_->queue_emtpy_interval_);
        }
    }


    /**
     * �еȴ���ִ������
     * @param ms
     * @return
     * @notice Ŀ���ǽ���cpu��ռ����
     */
    void waitRunTask(long ms) {
        auto task = this->pool_task_queue_->popWithTimeout(ms);
        if (nullptr != task) {
            (*task)();
        }
    }


    /**
     * �жϱ��߳��Ƿ���Ҫ���Զ��ͷ�
     * @return
     */
    bool freeze() {
        if (likely(is_running_)) {
            cur_ttl_++;
            cur_ttl_ = std::min(cur_ttl_, config_->secondary_thread_ttl_);
        }
        else {
            cur_ttl_--;    // �����ǰ�߳�û����ִ�У���ttl-1
        }

        return cur_ttl_ <= 0 && done_;    // ����������ִ�е��̣߳��ſ��Ա�����
    }

private:
    long cur_ttl_ = 0;                                                      // ��ǰ�����������

    friend class ThreadPool;
};

using ThreadSecondaryPtr = ThreadSecondary*;

}

#endif
