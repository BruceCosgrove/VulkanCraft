#pragma once

#include "Engine/Logging/Log.hpp"
#include "Engine/Input/Window.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"

namespace eng
{
    struct ApplicationInfo
    {
        WindowInfo WindowInfo;
    };

    class Application
    {
    public:
        static Application& Get(); // Defined in Entrypoint.cpp

    private:
        void OnEvent(Event& event);
        void OnWindowCloseEvent(WindowCloseEvent& event);

    private:
        Window m_Window;
        bool m_Running = false;

    private:
        friend int Main(int argc, char** argv);

        Application(ApplicationInfo const& info);
        ~Application();
        void Run();
    };
}
