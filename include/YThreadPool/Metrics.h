
#ifndef METRICS_H
#define METRICS_H

#include <iostream>
#include <string>

namespace YThreadPool{

class Metrics {
protected:
    explicit Metrics() = default;

    /**
     * ����thread ����ץȡ����Ϣ
     * @param result
     * @param size
     * @return
     */
    inline void calcLocal(bool result, size_t size) {
#ifndef _CGRAPH_SHOW_THREAD_METRICS_
        return;
#endif
        if (result) {
            local_pop_real_num_ += size;
        }
        local_pop_times_++;
    }

    /**
     * ����thread ��ȫ�ֶ�����ץȡ����Ϣ
     * @param result
     * @param size
     * @return
     */
    inline void calcPool(bool result, size_t size) {
#ifndef _CGRAPH_SHOW_THREAD_METRICS_
        return;
#endif
        if (result) {
            pool_pop_real_num_ += size;
        }
        pool_pop_times_++;
    }

    /**
     * ����thread ��͵������Ϣ
     * @param result
     * @param size
     * @return
     */
    inline void calcSteal(bool result, size_t size) {
#ifndef _CGRAPH_SHOW_THREAD_METRICS_
        return;
#endif
        if (result) {
            steal_pop_real_num_ += size;
        }
        steal_pop_times_++;
    }

    /**
     * չʾ��Ӧ�Ľ����Ϣ
     * @param key
     * @return
     * @notice ��ȷ������������ȫ��ȷ����Ҫ�Ǳ��������ܵ��ŷ�����ʹ��
     */
    void show(const std::string& tag) const {
#ifndef _CGRAPH_SHOW_THREAD_METRICS_
        return;
#endif
        std::cout << tag
            << ": local_pop_real_num_ " << local_pop_real_num_
            << ", local_pop_times_ " << local_pop_times_
            << ", pool_pop_real_num_ " << pool_pop_real_num_
            << ", pool_pop_times_ " << pool_pop_times_
            << ", steal_pop_real_num_ " << steal_pop_real_num_
            << ", steal_pop_times_ " << steal_pop_times_
            << "\n \t fleet_wait_times_ " << fleet_wait_times_
            << ", deep_wait_times_ " << deep_wait_times_
            << ", local_push_real_num_ " << local_push_real_num_
            << ", local_push_yield_times_ " << local_push_yield_times_
            << "\n \t ";

        float localSuccessRate = local_pop_times_ > 0 ? float(local_pop_real_num_) / float(local_pop_times_) : 0.0f;
        float poolSuccessRate = pool_pop_times_ > 0 ? float(pool_pop_real_num_) / float(pool_pop_times_) : 0.0f;
        float stealSuccessRate = steal_pop_times_ > 0 ? float(steal_pop_real_num_) / float(steal_pop_times_) : 0.0f;
        float pushYieldRate = local_push_yield_times_ > 0 ? float(local_push_yield_times_) / float(local_push_real_num_) : 0.0f;

        printf("local success rate is [%0.3f], pool success rate is [%0.3f], "    \
            "steal success rate is [%0.3f], push yield rate is [%0.4f] \n\n", \
            localSuccessRate, poolSuccessRate, stealSuccessRate, pushYieldRate);
    }

    /**
     * �ָ����е�������Ϣ
     * @return
     */
    void reset() {
        local_pop_real_num_ = 0;
        local_pop_times_ = 0;
        pool_pop_real_num_ = 0;
        pool_pop_times_ = 0;
        steal_pop_real_num_ = 0;
        steal_pop_times_ = 0;
        local_push_real_num_ = 0;
        local_push_yield_times_ = 0;
        fleet_wait_times_ = 0;
        deep_wait_times_ = 0;
    }

private:
    size_t local_pop_real_num_ = 0;        // ����pop�������ݸ���
    size_t local_pop_times_ = 0;           // ���س���pop�Ĵ���
    size_t pool_pop_real_num_ = 0;         // ��pool��pop���������ݸ���
    size_t pool_pop_times_ = 0;            // ��pool�г���pop�Ĵ���
    size_t steal_pop_real_num_ = 0;        // ͵����ȡ�����ݵĸ���
    size_t steal_pop_times_ = 0;           // ͵���Ĵ���
    size_t local_push_real_num_ = 0;       // д�����ʵ����
    size_t local_push_yield_times_ = 0;    // д���ͻ����yield�Ĵ���
    size_t fleet_wait_times_ = 0;          // �������ȴ��Ĵ���
    size_t deep_wait_times_ = 0;           // ��ȵȴ��Ĵ�����������cv��wait���ƣ�

    friend class ThreadBase;
    friend class ThreadPrimary;
    friend class ThreadSecondary;
    friend class ThreadPool;
};

}

#endif