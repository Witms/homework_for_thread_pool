#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <condition_variable>
#include <memory>

class ThreadPool;


void quicksort(int* array, long left, long right, ThreadPool& pool, std::promise<void> *);
// ������� ����������� ��� ���������� ����
typedef std::function<void()> task_type;
// ��� ��������� �� �������, ������� �������� �������� ��� ������� �����
typedef void (*FuncType) (int*, long, long, ThreadPool& pool, std::promise<void>*);


template<class T>
class BlockedQueue {
public:
    void push(T& item) {
        std::lock_guard<std::mutex> l(m_locker);
        // ������� ���������������� push
        m_task_queue.push(item);
        // ������ ����������, ����� �����, ���������
        // pop ��������� � ������ ������� �� �������
        m_notifier.notify_one();
    }
    // ����������� ����� ��������� �������� �� �������
    void pop(T& item) {
        std::unique_lock<std::mutex> l(m_locker);
        if (m_task_queue.empty())
            // ����, ���� ������� push
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }
    // ������������� ����� ��������� �������� �� �������
    // ���������� false, ���� ������� �����
    bool fast_pop(T& item) {
        std::lock_guard<std::mutex> l(m_locker);
        if (m_task_queue.empty())
            // ������ �������
            return false;
        // �������� �������
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }
private:
    std::mutex m_locker;
    // ������� �����
    std::queue<T> m_task_queue;
    // �����������
    std::condition_variable m_notifier;
};


class ThreadPool {
public:
    ThreadPool();
    // ������:
    void start();
    // ���������:
    void stop();
    // ������� �����
    void push_task(FuncType f, int*, long, long, ThreadPool& pool, std::promise<void>* pro = nullptr);
    // ������� ����� ��� ������
    void threadFunc(int qindex);
private:
    // ���������� �������
    int m_thread_count;
    // ������
    std::vector<std::thread> m_threads;
    // ������� ����� ��� �������
    std::vector<BlockedQueue<task_type>> m_thread_queues;
    // ��� ������������ ������������� �����
    int m_index;
};

