
#include "YThreadPool/ThreadPool.h"

namespace YThreadPool{

ThreadPool::ThreadPool(bool autoInit, const ThreadPoolConfig& config) noexcept {
    cur_index_ = 0;
    is_init_ = false;
    this->setConfig(config);    // setConfig ���������� is_init_ �趨֮��
    if (autoInit) {
        this->init();
    }
}


ThreadPool::~ThreadPool() {
    this->config_.monitor_enable_ = false;    // ��������ʱ�򣬲��ͷż���̡߳����ͷż���̣߳����ͷ��������߳�
    if (monitor_thread_.joinable()) { //����߳��Ƿ������
        monitor_thread_.join();
    }

    destroy();
}


std::string ThreadPool::setConfig(const ThreadPoolConfig& config) {
    std::string res;
    this->config_ = config;
    return res;
}


std::string ThreadPool::init() {
    std::string res;
        if (is_init_) {
            return res;
        }

    if (config_.monitor_enable_) {
        // Ĭ�ϲ���������߳�
        monitor_thread_ = std::move(std::thread(&ThreadPool::monitor, this));
    }
    thread_record_map_.clear();
    thread_record_map_[(size_t)std::hash<std::thread::id>{}(std::this_thread::get_id())] = MAIN_THREAD_ID;//��ǰ�̵߳�ID�Ĺ�ϣֵ���������̱߳��
    primary_threads_.reserve(config_.default_thread_size_);
    for (int i = 0; i < config_.default_thread_size_; i++) {
        ThreadPrimary* pt = nullptr;
        while (!pt) {
            pt = new(std::nothrow) ThreadPrimary();// ���������߳���
        }
        pt->setThreadPoolInfo(i, &task_queue_, &primary_threads_, &config_);
        // ��¼�̺߳�ƥ��id��Ϣ
        primary_threads_.emplace_back(pt);
    }

    /**
     * �ȴ�����thread �������֮���ٽ��� init()��
     * �����ڸ����ƽ̨�ϣ����ܳ��� thread�������������̡߳����ҵ����쳣�����
     * �ο��� https://github.com/ChunelFeng/CGraph/issues/309
     */
    for (int i = 0; i < config_.default_thread_size_; i++) {
        res += primary_threads_[i]->init();
        thread_record_map_[(size_t)std::hash<std::thread::id>{}(primary_threads_[i]->thread_.get_id())] = i;
    }
    if (!res.empty())std::cout << res << std::endl;

        /**
         * ���Ը��£�
         * ��ʼ����ʱ��Ҳ���Դ���n�������̡߳�Ŀ����Ϊ����Ͻ�ʹ�� pool�� priority_queue �ĳ���
         * һ������£�����Ϊ0��
         */
    res = createSecondaryThread(config_.secondary_thread_size_);
    if (!res.empty())std::cout << res << std::endl;

    is_init_ = true;
    return res;
}


std::string ThreadPool::submit(const TaskGroup& taskGroup, long ttl) {
    std::string res;
     
    std::vector<std::future<void>> futures;
    for (const auto& task : taskGroup.task_arr_) {
        futures.emplace_back(commit(task));
    }

    // ������������ʱ����Ϣ
    auto deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(std::min(taskGroup.getTtl(), ttl));

    for (auto& fut : futures) {
        const auto& futStatus = fut.wait_until(deadline);//������ǰ�̵߳�ָ���Ľ�ֹʱ�䣬�ȴ��������(�����ͷ���) 
        switch (futStatus) {
        case std::future_status::ready: break;    // ���������ֱ�ӷ�����
        case std::future_status::timeout: res += "thread status timeout"; break;
        case std::future_status::deferred: res += "thread status deferred"; break;
        default: res += "thread status unknown";
        }
    }

    if (taskGroup.on_finished_) {
        taskGroup.on_finished_(res);
    }

    return res;
}


std::string ThreadPool::submit(const std::function<void()>& func, long ttl,
    const std::function<void(std::string)>& onFinished) {
    return submit(TaskGroup(func, ttl, onFinished));
}


int ThreadPool::getThreadIndex(size_t tid) {
    int index = SECONDARY_THREAD_COMMON_ID;
    auto result = thread_record_map_.find(tid);
    if (result != thread_record_map_.end()) {
        index = result->second;
    }

    return index;
}


std::string ThreadPool::destroy() {
    std::string res;
    if (!is_init_) {
        return res;
    }

    // primary �߳�����ָͨ�룬��Ҫdelete
    for (auto& pt : primary_threads_) {
        res += pt->destroy();
    }
    if (!res.empty())std::cout << res << std::endl;

        /**
         * ����֮���� destroy�� delete�ֿ�����ѭ��ִ�У�
         * ����Ϊ��ǰ�̱߳�delete�󣬻����ܴ���δ��delete�����̣߳���steal��ǰ�̵߳�����
         * ��windows�����£����ܳ������⡣
         * destroy �� delete �ֿ�֮�󣬲�����ִ����⡣
         * ��л Ryan����(https://github.com/ryanhuang) �ṩ�İ���
         */
        for (auto& pt : primary_threads_) {
            delete pt;  
            pt = nullptr;
        }
    primary_threads_.clear();
    // secondary �߳�������ָ�룬����Ҫdelete
    for (auto& st : secondary_threads_) {
        res += st->destroy();
    }
    
    secondary_threads_.clear();
    thread_record_map_.clear();
    is_init_ = false;

    return res;
}


bool ThreadPool::isInit() const {
    return is_init_;
}


std::string ThreadPool::releaseSecondaryThread(int size) {
    std::string res;

    // �Ƚ������Ѿ������ģ���ɾ��
    std::lock_guard<std::mutex> lock(st_mutex_);
    for (auto iter = secondary_threads_.begin(); iter != secondary_threads_.end(); ) {
        !(*iter)->done_ ? secondary_threads_.erase(iter++) : iter++;
    }


    if (unlikely(size > secondary_threads_.size())) {
        res += "cannot release [" + std::to_string(size) + "] secondary thread," \
            + "only [" + std::to_string(secondary_threads_.size()) + "] left.";
    }

        // �ٱ�Ǽ�����Ҫɾ������Ϣ
    for (auto iter = secondary_threads_.begin();iter != secondary_threads_.end() && size-- > 0; ) {
        (*iter)->done_ = false;
        iter++;
    }

    return res;
}


int ThreadPool::dispatch(int origIndex) { //�򵥵ĸ��ؾ������
    int realIndex = 0;
    if (DEFAULT_TASK_STRATEGY == origIndex) {
        /**
         * �����Ĭ�ϲ�����Ϣ����[0, default_thread_size_) ֮��ģ�ͨ�� thread ��queue������
         * ��[default_thread_size_, max_thread_size_) ֮��ģ�ͨ�� pool �е�queue������
         */
        realIndex = cur_index_++;
        if (cur_index_ >= config_.max_thread_size_ || cur_index_ < 0) {
            cur_index_ = 0;
        }
    }
    else {
        realIndex = origIndex;
    }

    return realIndex;    // ��������ȥ�жϣ����ĸ��߳�
}


std::string ThreadPool::createSecondaryThread(int size) {
    std::string res;

    int leftSize = (int)(config_.max_thread_size_ - config_.default_thread_size_ - secondary_threads_.size());
    int realSize = std::min(size, leftSize);    // ʹ�� realSize ��ȷ�����е��߳�����֮�ͣ����ᳬ���趨maxֵ

    std::lock_guard<std::mutex> lock(st_mutex_);
    for (int i = 0; i < realSize; i++) {
        auto ptr = std::make_unique<ThreadSecondary>();
        ptr->setThreadPoolInfo(&task_queue_, &priority_task_queue_, &config_);
        res += ptr->init();
        secondary_threads_.emplace_back(std::move(ptr));
    }

    return res;
}


void ThreadPool::monitor() {
    while (config_.monitor_enable_) {
        while (config_.monitor_enable_ && !is_init_) {
            // ���û��init����һֱ���ڿ���״̬
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        auto span = config_.monitor_span_;
        while (config_.monitor_enable_ && is_init_ && span--) {
            std::this_thread::sleep_for(std::chrono::seconds(1));    // ��֤���Կ����˳�
        }

        // ��� primary�̶߳���ִ�У����ʾæµ
        bool busy = !primary_threads_.empty() && std::all_of(primary_threads_.begin(), primary_threads_.end(),
            [](ThreadPrimaryPtr ptr) { return nullptr != ptr && ptr->is_running_; });

        std::lock_guard<std::mutex> lock(st_mutex_);
        // ���æµ����priority_task_queue_������������Ҫ��� secondary�߳�
        if (busy || !priority_task_queue_.empty()) {
            createSecondaryThread(1);
        }

        // �ж� secondary �߳��Ƿ���Ҫ�˳�
        for (auto iter = secondary_threads_.begin(); iter != secondary_threads_.end(); ) {
            (*iter)->freeze() ? secondary_threads_.erase(iter++) : iter++;
        }
    }
}

}