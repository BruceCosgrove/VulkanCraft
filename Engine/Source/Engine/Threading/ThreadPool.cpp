#include "ThreadPool.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Threading/ThreadTracer.hpp"

namespace eng
{
    ThreadPool::ThreadPool(u8 threadCount, bool finishAllTasksOnDestroy)
        : m_FinishAllTasksOnDestroy(finishAllTasksOnDestroy)
    {
        ENG_LOG_TRACE("Starting thread pool with {} threads.", threadCount);

        m_Threads.reserve(threadCount);
        for (u8 i = 0; i < threadCount; i++)
            m_Threads.emplace_back([this] { ThreadFunction(); });
    }

    ThreadPool::~ThreadPool()
    {
        ENG_LOG_TRACE("Exiting thread pool with {} threads.", m_Threads.size());

        m_Running.store(false, std::memory_order_relaxed);
        m_TaskAvailable.notify_all();
    }

    void ThreadPool::SubmitTask(std::function<void()>&& task)
    {
        ENG_LOG_TRACE("Submiting task to thread pool.");

        {
            std::unique_lock lock(m_TaskMutex);
            m_Tasks.push_back(std::move(task));
        }
        m_TaskAvailable.notify_one();
    }

    void ThreadPool::ThreadFunction()
    {
        ThreadTracer tracer("pooled");

        while (true)
        {
            // Acquire a task to perform.
            std::function<void()> task;
            {
                bool running = true, tasksRemaining = true;
                std::unique_lock lock(m_TaskMutex);
                while (not (tasksRemaining = not m_Tasks.empty()) and (running = m_Running.load(std::memory_order_relaxed)))
                    m_TaskAvailable.wait(lock);

                if (not tasksRemaining or not (running or m_FinishAllTasksOnDestroy))
                    break;
                task = std::move(m_Tasks.front());
                m_Tasks.pop_front();
            }

            // Perform the task.
            ENG_LOG_TRACE("Starting task on pooled thread with tid={}.", tracer.GetTID()._Get_underlying_id());
            task();
            ENG_LOG_TRACE("Finishing task on pooled thread with tid={}.", tracer.GetTID()._Get_underlying_id());
        }
    }
}
