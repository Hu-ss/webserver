#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include <exception>
#include <cstdio>
#include "locker.h"

// 线程池类，定义成模板类是为了代码的复用，模板参数T是任务类
template <typename T>
class threadpool
{
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    // 添加任务
    bool append(T *request);

private:
    static void *worker(void *arg);
    void run();

private:
    // 线程的数量
    int m_thread_number;

    // 线程池数组，大小为m_thread_number
    pthread_t *m_threads;

    // 请求队列中最多允许的，等待处理的请求数量
    int m_max_requests;

    // 请求队列
    std::list<T *> m_workqueue;

    // 互斥锁
    locker m_queuelocker;

    // 信号量用来判断是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程
    bool m_stop;
};

// 实现构造函数
template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests),
                                                                 m_stop(false), m_threads(NULL)
{

    if ((thread_number <= 0) || (max_requests <= 0))
    {
        throw std::exception();
    }

    // 创建数组
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
    {
        throw std::exception();
    }

    // 创建thread_number个线程并将他们设置为线程脱离
    for (int i = 0; i < thread_number; ++i)
    {
        printf("create the %dth thread\n", i);

        // 创建子线程
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }

        // 子线程分离
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

// 实现析构函数
template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

template <typename T>
bool threadpool<T>::append(T *request)
{

    // 上锁，防止其他线程对当前变量进行修改
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request); // 将请求加入到请求队列中
    m_queuelocker.unlock();         // 解锁
    m_queuestat.post();             // 信号量+1
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        // 判断是否有信号量，有信号量则信号量-1
        m_queuestat.wait();
        // 有信号量就上锁
        m_queuelocker.lock();
        // 如果请求队列是空，则没有需要执行的任务，解锁
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        // 取出请求队列头部的任务
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if (!request)
        {
            continue;
        }
        // 对取出的任务进行处理
        request->process();
    }
}

#endif