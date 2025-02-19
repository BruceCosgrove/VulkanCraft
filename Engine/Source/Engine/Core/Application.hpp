#pragma once

#include "Engine/Core/LayerStack.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Input/Window.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"

namespace eng
{
    struct ApplicationInfo
    {
        // TODO: Support any non-negative number of windows;
        // both multiple window support and headless mode, e.g. for server hosting.
        WindowInfo WindowInfo;
    };

    class Application
    {
    public:
        // Global accessor for the application instance.
        static Application& Get(); // Defined in Entrypoint.cpp

        // Shuts down the application gracefully.
        void Terminate() noexcept;

        LayerStack& GetLayerStack() noexcept;
    private:
        void OnEvent(Event& event);
        void OnWindowCloseEvent(WindowCloseEvent& event);
    private:
        LayerStack m_LayerStack;
        Window m_Window;
        bool m_Running = true;
        double m_LastTime = 0.0;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application();
        void Run();
    };
}
