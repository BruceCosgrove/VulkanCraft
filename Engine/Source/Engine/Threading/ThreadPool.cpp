#include "ThreadPool.hpp"
#include "Engine/Core/Log.hpp"

namespace eng
{
    ThreadPool::ThreadPool(u8 count)
    {
        ENG_LOG_TRACE("Starting thread pool with {} threads.", count);

        m_Threads.reserve(count);
        for (u8 i = 0; i < count; i++)
            m_Threads.emplace_back([this] { ThreadFunction(); });
    }

    ThreadPool::~ThreadPool()
    {
        ENG_LOG_TRACE("Exiting thread pool with {} threads.", m_Threads.size());

        m_Terminating.store(true, std::memory_order_release);
        m_TaskAvailable.notify_all();
    }

    void ThreadPool::SubmitTask(std::function<void()>&& task)
    {
        ENG_LOG_TRACE("Submiting task to thread pool.");

        {
            std::scoped_lock lock(m_TaskMutex);
            m_Tasks.push_back(std::move(task));
        }
        m_TaskAvailable.notify_all();
    }

    void ThreadPool::ThreadFunction()
    {
        u32 tid = std::this_thread::get_id()._Get_underlying_id();
        ENG_LOG_TRACE("Pooled thread with tid={} starting.", tid);

        while (true)
        {
            // Acquire a task to perform.
            std::function<void()> task;
            {
                std::unique_lock lock(m_TaskMutex);
                bool terminating = false;
                while (not (terminating = m_Terminating.load(std::memory_order_acquire)) and m_Tasks.empty())
                    m_TaskAvailable.wait(lock);
                if (terminating)
                    break;
                task = std::move(m_Tasks.front());
                m_Tasks.pop_front();
            }

            // Perform the task.
            ENG_LOG_TRACE("Pooled thread with tid={} starting task.", tid);
            task();
            ENG_LOG_TRACE("Pooled thread with tid={} finished task.", tid);
        }

        ENG_LOG_TRACE("Pooled thread with tid={} exiting.", tid);
    }
}
