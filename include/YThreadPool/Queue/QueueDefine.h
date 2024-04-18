#ifndef QUEUEDEFINE_H
#define QUEUEDEFINE_H

namespace YThreadPool{

/** �����ζ�������ʱ��д����Ϣʱ��Ĳ��� */
enum class RingBufferPushStrategy {
    WAIT = 1,                 // �ȴ������ݱ����Ѻ���д��
    REPLACE = 2,              // �滻δ�����ѵ�������������
    DROP = 3,                 // ������ǰ��Ϣ
};

}

#endif 
