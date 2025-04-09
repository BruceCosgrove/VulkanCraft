#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/Window.hpp"
#include "Engine/Threading/ThreadPool.hpp"

namespace eng
{
    struct ApplicationInfo
    {
        // TODO: Support any non-negative number of windows;
        // both multiple window support and headless mode, e.g. for server hosting.
        // This should include a separate headless build.
        WindowInfo WindowInfo;
        // Set to 0 to disable.
        u8 ThreadPoolSize = 4;
    };

    class Application
    {
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Application);
    public:
        // Global accessor for the application instance.
        // May be called from any thread.
        static Application& Get(); // Defined in Entrypoint.cpp

        // Shuts down the application gracefully.
        // May be called from any thread.
        void Terminate();

        // Returns if the application is still running.
        // May be called from any thread.
        bool IsRunning() const;

        // Execute a task in the global thread pool.
        // May be called from any thread.
        void ExecuteAsync(std::function<void()>&& task);
    private:
        void UpdateThread();
        void RenderThread();
        void EventThread();
    private:
        std::atomic_bool m_Running = true;
        ThreadPool m_ThreadPool;
        Window m_Window;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application() = default;
        void Run();
    };
}
