#include "thread.h"

ThreadPool::ThreadPool(size_t numThreads) : stop(false)
{
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {   
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock,
                        [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task(); 
            }
            });
    }
}

ThreadPool::~ThreadPool()
{
    {   
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (auto& t : workers) t.join();
}

void ThreadPool::enqueue(std::function<void()> job)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(std::move(job));
    }
    condition.notify_one();
}
