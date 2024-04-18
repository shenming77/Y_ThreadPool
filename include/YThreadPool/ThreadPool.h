

#ifndef YTHREADPOOL_H
#define YTHREADPOOL_H

#include <vector>
#include <list>
#include <map>
#include <future>
#include <thread>
#include <algorithm>
#include <memory>
#include <functional>

#include "YThreadPool/Queue/AtomicQueue.h"
#include "YThreadPool/Queue/WorkStealingQueue.h"
#include "YThreadPool/Queue/AtomicPriorityQueue.h"
#include "YThreadPool/Queue/AtomicRingBufferQueue.h"
#include "YThreadPool/Queue/LockFreeRingBufferQueue.h"
#include "YThreadPool/Task/Task.h"
#include "YThreadPool/Task/TaskGroup.h"
#include "YThreadPool/ThreadPoolConfig.h"
#include "YThreadPool/Thread/ThreadPrimary.h"
#include "YThreadPool/Thread/ThreadSecondary.h"

namespace YThreadPool{

class ThreadPool {
public:
    /**
     * ͨ��Ĭ�����ò������������̳߳�
     * @param autoInit �Ƿ��Զ������̳߳ع���
     * @param config
     */
    explicit ThreadPool(bool autoInit = true,
        const ThreadPoolConfig& config = ThreadPoolConfig()) noexcept;

    /**
     * ��������
     */
    ~ThreadPool();

    /**
     * �����̳߳����������Ϣ����Ҫ��init()��������ǰ���������
     * @param config
     * @return
     * @notice ͨ��������(ThreadPoolSingleton)�����̳߳أ����̳߳�Ĭ��init����Ҫ destroy ��ſ������ò���
     */
    std::string setConfig(const ThreadPoolConfig& config);

    /**
     * �������е��߳���Ϣ
     * @return
     */
    std::string init();

    /**
     * �ύ������Ϣ
     * @tparam FunctionType
     * @param task
     * @param index
     * @return
     */
    template<typename FunctionType>
    auto commit(const FunctionType& task,
        int index = DEFAULT_TASK_STRATEGY)
        -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * ���ض����߳�id�У��ύ������Ϣ
     * @tparam FunctionType
     * @param task
     * @param tid �߳�id������������̸߳�����Χ����Ĭ��д��pool��ͨ�ö�����
     * @param enable �Ƿ���������/��������
     * @param lockable ����(true) / ����(false)
     * @return
     */
    template<typename FunctionType>
    auto commitWithTid(const FunctionType& task, int tid, bool enable, bool lockable)
        -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * �������ȼ���ִ������
     * @tparam FunctionType
     * @param task
     * @param priority ���ȼ�����Ȼ��Ӵ�С����ִ��
     * @return
     * @notice ���飬priority ��Χ�� [-100, 100] ֮��
     */
    template<typename FunctionType>
    auto commitWithPriority(const FunctionType& task,
        int priority)
        -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * ִ����������Ϣ
     * ȡtaskGroup�ڲ�ttl�����ttl����Сֵ��Ϊ����ttl��׼
     * @param taskGroup
     * @param ttl
     * @return
     */
    std::string submit(const TaskGroup& taskGroup,
        long ttl = MAX_BLOCK_TTL);

    /**
     * ��Ե�������������������������Ϣ��ʵ�ֵ�������ֱ��ִ��
     * @param task
     * @param ttl
     * @param onFinished
     * @return
     */
    std::string submit(const std::function<void()>& func,
        long ttl = MAX_BLOCK_TTL,
        const std::function<void(std::string)>& onFinished = nullptr);

    /**
     * ��ȡ�����߳�id��Ϣ����ȡ�߳�index��Ϣ
     * @param tid
     * @return
     * @notice �����̷߳���-1
     */
    int getThreadIndex(size_t tid);

    /**
     * �ͷ����е��߳���Ϣ
     * @return
     */
    std::string destroy();

    /**
     * �ж��̳߳��Ƿ��Ѿ���ʼ����
     * @return
     */
    bool isInit() const;

    /**
     * ���ɸ����̡߳��ڲ�ȷ�������߳������������趨����
     * @param size
     * @return
     */
    std::string createSecondaryThread(int size);

    /**
     * ɾ�������߳�
     * @param size
     * @return
     */
    std::string releaseSecondaryThread(int size);

protected:
    /**
     * ���ݴ���Ĳ�����Ϣ��ȷ������ִ�з�ʽ
     * @param origIndex
     * @return
     */
    virtual int dispatch(int origIndex);

    /**
     * ����߳�ִ�к�������Ҫ���ж��Ƿ���Ҫ�����̣߳��������߳�
     * ��/ɾ �����������secondary�����߳���Ч
     */
    void monitor();

    ThreadPool(const ThreadPool&) = delete;
    const ThreadPool& operator=(const ThreadPool&) = delete;

private:
    bool is_init_{ false };                                                       // �Ƿ��ʼ��
    int cur_index_ = 0;                                                            // ��¼������߳���
    AtomicQueue<Task> task_queue_;                                                // ���ڴ����ͨ����
    AtomicPriorityQueue<Task> priority_task_queue_;                               // ����ʱ��ϳ���������У����ڸ����߳���ִ��
    std::vector<ThreadPrimaryPtr> primary_threads_;                                // ��¼���е����߳�
    std::list<std::unique_ptr<ThreadSecondary>> secondary_threads_;                // ���ڼ�¼���еĸ����߳�
    ThreadPoolConfig config_;                                                      // �̳߳�����ֵ
    std::thread monitor_thread_;                                                    // ����߳�
    std::map<size_t, int> thread_record_map_;                                        // �̼߳�¼����Ϣ
    std::mutex st_mutex_;                                                           // �����̷߳����䶯��ʱ�򣬼ӵ�mutex��Ϣ
};

using ThreadPoolPtr = std::unique_ptr<ThreadPool>;
using ThreadPoolPtrs = std::shared_ptr<ThreadPool>;

}

#include "ThreadPool.inl"


#endif 
