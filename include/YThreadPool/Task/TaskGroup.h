//�����飬���������ύ

#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <utility>
#include <functional>
#include <string>
#include <vector>


#include "YThreadPool/ThreadPoolDefine.h"

namespace YThreadPool{

class TaskGroup {
public:
    explicit TaskGroup() = default;

    TaskGroup(const TaskGroup&) = delete; 
    const TaskGroup& operator=(const TaskGroup&) = delete;
        /**
         * ֱ��ͨ������������taskGroup
         * @param task
         * @param ttl
         * @param onFinished
         */
    explicit TaskGroup(const std::function<void()>& task,
            long ttl = MAX_BLOCK_TTL,
            const std::function<void(std::string)>& onFinished = nullptr) noexcept {
            this->addTask(task)
            ->setTtl(ttl)
            ->setOnFinished(onFinished);
    }

    /**
     * ���һ������
     * @param task
     */
    TaskGroup* addTask(const std::function<void()>& task) {
        task_arr_.emplace_back(task);
        return this;
    }

    /**
     * �����������ʱʱ��
     * @param ttl
     */
    TaskGroup* setTtl(long ttl) {
        this->ttl_ = ttl;
        return this;
    }

    /**
     * ����ִ����ɺ�Ļص�����
     * @param onFinished
     * @return
     */
    TaskGroup* setOnFinished(const std::function<void(std::string)>& onFinished) {
        this->on_finished_ = onFinished;
        return this;
    }

    /**
     * ��ȡ���ʱʱ����Ϣ
     * @return
     */
    long getTtl() const {
        return this->ttl_;
    }

    /**
     * ���������
     */
    void clear() {
        task_arr_.clear();
    }

    /**
     * ��ȡ�������С
     * @return
     */
    size_t getSize() const {
        auto size = task_arr_.size();
        return size;
    }

private:
    std::vector<std::function<void()>> task_arr_;         // ������Ϣ
    long ttl_ = MAX_BLOCK_TTL;                      // ���������ִ�к�ʱ(�����0�Ļ������ʾ������)
    std::function<void(std::string)> on_finished_ = nullptr;        // ִ�к����������

    friend class ThreadPool;
};

using TaskGroupPtr = TaskGroup*;
using TaskGroupRef = TaskGroup&;

}

#endif 
