#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Input/Window.hpp"

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
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Application);

        // Global accessor for the application instance.
        static Application& Get(); // Defined in Entrypoint.cpp

        // Shuts down the application gracefully.
        void Terminate();
    private:
        bool m_Running = true;
        Window m_Window;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application() = default;
        void Run();
    };
}
