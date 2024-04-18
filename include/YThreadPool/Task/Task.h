
#ifndef TASK_H
#define TASK_H

#include <vector>
#include <memory>
#include <type_traits>

namespace YThreadPool{

class Task  {
    struct TaskBased {
        explicit TaskBased() = default;
        virtual void call() = 0;
        virtual ~TaskBased() = default;
    };

    // �˻��Ի��ʵ�����ͣ��޸�˼·�ο���https://YThreadPool/nelFeng/C  YThreadPool/pull/3
    template<typename F, typename T = typename std::decay<F>::type>   // T = F �ġ�˥��"��decayed���汾
    struct TaskDerided : TaskBased {
        T func_;
        explicit TaskDerided(F&& func) : func_(std::forward<F>(func)) {}
        void call() final { func_(); }
    };

public:
    template<typename F>
    Task(F&& func, int priority = 0)
        : impl_(new TaskDerided<F>(std::forward<F>(func)))
        , priority_(priority) {}

    void operator()() {
        // impl_ �����ϲ�����Ϊ��
        impl_ ? impl_->call() : throw ("Task inner function is nullptr");
    }

    Task() = default;

    Task(Task&& task) noexcept :
        impl_(std::move(task.impl_)),
        priority_(task.priority_) {}

    Task& operator=(Task&& task) noexcept {
        impl_ = std::move(task.impl_);
        priority_ = task.priority_;
        return *this;
    }

    bool operator>(const Task& task) const {
        return priority_ < task.priority_;    // �¼���ģ��ŵ�����
    }

    bool operator<(const Task& task) const {
        return priority_ >= task.priority_;
    }

    Task(const Task&) = delete;                                      
    const Task& operator=(const Task&) = delete;

private:
    std::unique_ptr<TaskBased> impl_ = nullptr;
    int priority_ = 0;                                 // ��������ȼ���Ϣ
};


using TaskRef = Task&;
using TaskPtr = Task*;
using TaskArr = std::vector<Task>;
using TaskArrRef = std::vector<Task>&;

}

#endif 
