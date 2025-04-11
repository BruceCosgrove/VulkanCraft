#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Input/Window.hpp"
#include <atomic>

namespace eng
{
    struct ApplicationInfo
    {
        // TODO: Support any non-negative number of windows;
        // both multiple window support and headless mode, e.g. for server hosting.
        // This should include a separate headless build.
        WindowInfo WindowInfo;
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
    private:
        void UpdateThread();
        void RenderThread();
        void EventThread();
    private:
        std::atomic_bool m_Running = true;
        Window m_Window;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application() = default;
        void Run();
    };
}
