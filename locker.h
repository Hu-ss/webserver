#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
// 线程同步机制封装类

// 互斥锁类
class locker
{
public:
    locker()
    {
        // 初始化互斥锁
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }

    ~locker()
    {
        // 释放互斥锁
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock()
    {
        // 加锁
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock()
    {
        // 解锁
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t *get()
    {
        // 获取互斥量成员
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

// 条件变量类
class cond
{
public:
    cond()
    {
        // 初始化条件变量
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            throw std::exception();
        }
    }

    ~cond()
    {
        // 释放条件变量
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t *mutex)
    {
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }

    bool timedwait(pthread_mutex_t *mutex, struct timespec t)
    {
        return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
    }

    bool signal()
    {
        // 将一个线程唤醒
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast()
    {
        // 将所有线程唤醒
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};

// 信号量类
class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }

    sem(int num)
    {
        // 初始化信号量
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }

    ~sem()
    {
        // 释放信号量
        sem_destroy(&m_sem);
    }

    // 等待信号量
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }

    // 增加信号量
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

#endif
