#ifndef THREADPOOLDEFINE_H
#define THREADPOOLDEFINE_H

#include <thread>
#if __cplusplus >= 201703L
#include <shared_mutex>
#else
# include <mutex>
#endif
#include <memory>

namespace YThreadPool{

#ifdef _ENABLE_LIKELY_
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely
#define unlikely
#endif

static const int CPU_NUM = (int)std::thread::hardware_concurrency();
static const int THREAD_TYPE_PRIMARY = 1;
static const int THREAD_TYPE_SECONDARY = 2;
#ifndef _WIN32
static const int THREAD_SCHED_OTHER = SCHED_OTHER;
static const int THREAD_SCHED_RR = SCHED_RR;
static const int THREAD_SCHED_FIFO = SCHED_FIFO;
#else
/** �̵߳��Ȳ��ԣ��ݲ�֧��windowsϵͳ */
static const int THREAD_SCHED_OTHER = 0;
static const int THREAD_SCHED_RR = 0;
static const int THREAD_SCHED_FIFO = 0;
#endif
static const int THREAD_MIN_PRIORITY = 0;                                           // �߳�������ȼ�
static const int THREAD_MAX_PRIORITY = 99;                                          // �߳�������ȼ�
static const long MAX_BLOCK_TTL = 1999999999;                                       // �������ʱ�䣬��λΪms
static const unsigned int DEFAULT_RINGBUFFER_SIZE = 64;                                     // Ĭ�ϻ��ζ��еĴ�С
static const int MAIN_THREAD_ID = -1;                                             // �����߳�id��ʶ�����������̣߳�
static const int SECONDARY_THREAD_COMMON_ID = -2;                                 // �����߳�ͳһid��ʶ
static const int DEFAULT_PRIORITY = 0;                                              // Ĭ�����ȼ�


static const int DEFAULT_TASK_STRATEGY = -1;                                         // Ĭ���̵߳��Ȳ���
static const int POOL_TASK_STRATEGY = -2;                                            // �̶���pool�еĶ��еĵ��Ȳ���
static const int LONG_TIME_TASK_STRATEGY = -101;                                     // ��ʱ��������Ȳ���

/**
 * ����Ϊ�̳߳�������Ϣ
 */
static const int DEFAULT_THREAD_SIZE = 8;                                            // Ĭ�Ͽ������̸߳���
static const int SECONDARY_THREAD_SIZE = 0;                                          // Ĭ�Ͽ��������̸߳���
static const int MAX_THREAD_SIZE = 16;                                               // ����̸߳���
static const int MAX_TASK_STEAL_RANGE = 2;                                           // ��ȡ�������ڷ�Χ
static const bool BATCH_TASK_ENABLE = false;                                         // �Ƿ�������������
static const int MAX_LOCAL_BATCH_SIZE = 2;                                           // ����ִ�б����������ֵ
static const int MAX_POOL_BATCH_SIZE = 2;                                            // ����ִ��ͨ���������ֵ
static const int MAX_STEAL_BATCH_SIZE = 2;                                           // ������ȡ�������ֵ
static const int PRIMARY_THREAD_BUSY_EPOCH = 10;                                     // ���߳̽���wait״̬����������ֵԽ����������Խ�ߣ�����ת������ҲԽ��
static const long PRIMARY_THREAD_EMPTY_INTERVAL = 50;                               // ���߳̽�������״̬��Ĭ��ʱ��
static const long SECONDARY_THREAD_TTL = 10;                                         // �����߳�ttl����λΪs
static const bool MONITOR_ENABLE = false;                                            // �Ƿ�����س���
static const long MONITOR_SPAN = 5;                                                  // ����߳�ִ�м������λΪs
static const long QUEUE_EMPTY_INTERVAL = 3;                                         // ����Ϊ��ʱ���ȴ���ʱ�䡣����Ը����̣߳���λΪms
static const bool BIND_CPU_ENABLE = false;                                           // �Ƿ�����cpuģʽ����������̣߳�
static const int PRIMARY_THREAD_POLICY = THREAD_SCHED_OTHER;                  // ���̵߳��Ȳ���
static const int SECONDARY_THREAD_POLICY = THREAD_SCHED_OTHER;                // �����̵߳��Ȳ���
static const int PRIMARY_THREAD_PRIORITY = THREAD_MIN_PRIORITY;               // ���̵߳������ȼ���ȡֵ��Χ0~99����ϵ��Ȳ���һ��ʹ�ã������鲻�˽�������ݵ�ͯЬ���޸ģ�
static const int SECONDARY_THREAD_PRIORITY = THREAD_MIN_PRIORITY;             // �����̵߳������ȼ���ͬ�ϣ�

}

#endif 
