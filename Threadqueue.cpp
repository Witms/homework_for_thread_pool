#include "Threadqueue.h"
#include <iostream>

ThreadPool::ThreadPool() :
   	m_thread_count(std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4),
	m_thread_queues(m_thread_count) {

}

void ThreadPool::start() {
	for (int i = 0; i < m_thread_count; i++) {
		m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
	}
}

void ThreadPool::threadFunc(int qindex) {
    while (true) {
        // обработка очередной задачи
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do) {
            return;
        }
        // выполняем задачу
        task_to_do();
    }
}

void ThreadPool::stop() {
    for (int i = 0; i < m_thread_count; i++) {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }
}

void ThreadPool::push_task(FuncType f, int* array, long left, long right, ThreadPool& pool, std::promise<void>* pro) {
    // вычисляем индекс очереди, куда положим задачу
    if (pro != nullptr)
    {
        std::cout << "Not nullptr\n";
    }
    if (pro == nullptr)
    {
        std::cout << "nullptr\n";
    }
    int queue_to_push = m_index++ % m_thread_count;
    // формируем функтор
    task_type task = [=, &pool] {f(array, left, right, pool, pro); }; // Здесь кладутся задачи
    // кладем в очередь
    m_thread_queues[queue_to_push].push(task);
}


void quicksort(int* array, long left, long right, ThreadPool& pool, std::promise<void> *pro = nullptr) {

   static std::shared_ptr<int> point = std::make_shared<int>(*array);
 //   std::cout << "Counter: " << point.use_count() << std::endl;

    if (left >= right)
    {
        if(pro != nullptr)
            pro->set_value();
        return;
    }
    long left_bound = left;
    long right_bound = right;

    long middle = array[(left_bound + right_bound) / 2];

    do {
        while (array[left_bound] < middle) {
            left_bound++;
        }
        while (array[right_bound] > middle) {
            right_bound--;
        }

        //Меняем элементы местами
        if (left_bound <= right_bound) {
            std::swap(array[left_bound], array[right_bound]);
            left_bound++;
            right_bound--;
        }
    } while (left_bound <= right_bound);

    if (right_bound - left > 3)
    {
 //       std::promise<void> p;
        pool.push_task(quicksort, array, left, right_bound, pool, pro);
        quicksort(array, left_bound, right, pool);
        if (point.use_count() == 1 && pro != nullptr)
        {
            std::future<void> f = pro->get_future();
            std::cout << "What\n";
 //           f.wait();
            f.wait_for(std::chrono::seconds(5));
            f.get();
        }
 //       std::future<void> f = p.get_future();
 //       f.get();
 //       if(pro != nullptr)
//        pro->set_value();
    }
    else {
        // запускаем обе части синхронно
        quicksort(array, left, right_bound, pool);
        quicksort(array, left_bound, right, pool);
    }
}
