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
        u8 ThreadPoolSize = 4;
    };

    class Application
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Application);

        // Global accessor for the application instance.
        static Application& Get(); // Defined in Entrypoint.cpp

        // Shuts down the application gracefully.
        void Terminate();

        // Execute a task in a thread pool.
        void ExecuteAsync(std::function<void()>&& task);
    private:
        bool m_Running = true;
        ThreadPool m_ThreadPool;
        Window m_Window;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application() = default;
        void Run();
    };
}
