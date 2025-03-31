#pragma once

#include "Engine/Core/DataTypes.hpp"
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace eng
{
    // Basic thread pool implementation.
    class ThreadPool
    {
    public:
        // Create count number of threads and start them.
        // They will all immediately become idle and wait for tasks.
        ThreadPool(u8 count);

        // Signal to all threads in this pool that they should
        // terminate as soon as all tasks are completed and waits
        // for them all to terminate.
        ~ThreadPool();

        // Add a task that any one of the threads in this pool can perform.
        void SubmitTask(std::function<void()>&& task);
    private:
        void ThreadFunction();
    private:
        std::vector<std::jthread> m_Threads;
        std::atomic_bool m_Running = true;

        std::mutex m_TaskMutex;
        std::condition_variable m_TaskAvailable;
        std::deque<std::function<void()>> m_Tasks;
    };
}
