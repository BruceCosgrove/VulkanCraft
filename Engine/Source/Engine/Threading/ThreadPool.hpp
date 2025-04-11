#pragma once

#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/ClassTypes.hpp"
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
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ThreadPool);
    public:
        // Create threadCount number of threads and start them.
        // They will all immediately become idle and wait for tasks.
        ThreadPool(u8 threadCount, bool finishAllTasksOnDestroy);

        // Signal to all threads in this pool that they should terminate as
        // soon as all tasks are completed and waits for them all to terminate.
        ~ThreadPool();

        // Add a task that any one of the threads in this pool can perform.
        //
        // NOTE: Do NOT submit a task that either resubmits itself or submits
        // other tasks that that submit each other and then the original task.
        // That will cause the application to never exit, and may also cause
        // a memory leak.
        void SubmitTask(std::function<void()>&& task);
    private:
        void ThreadFunction();
    private:
        std::vector<std::jthread> m_Threads;
        std::atomic_bool m_Running = true;
        bool m_FinishAllTasksOnDestroy;

        std::mutex m_TaskMutex;
        std::condition_variable m_TaskAvailable;
        std::deque<std::function<void()>> m_Tasks;
    };
}
