#pragma once

#include "Engine/Core/LayerStack.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Input/Window.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/Rendering/RenderContext.hpp"

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
        // Global accessor for the application instance.
        static Application& Get(); // Defined in Entrypoint.cpp

        // Shuts down the application gracefully.
        void Terminate() noexcept;

        LayerStack& GetLayerStack() noexcept;
        RenderContext& GetRenderContext() noexcept;
    private:
        void OnEvent(Event& event);
        void OnWindowCloseEvent(WindowCloseEvent& event);
        void OnWindowMinimizeEvent(WindowMinimizeEvent& event);
        void OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event);
    private:
        // IMPORTANT: The order of these members is critical for initialization and shutdown.

        LayerStack m_LayerStack;
        Window m_Window;
        RenderContext m_RenderContext;
    private:
        double m_LastTime = 0.0;
        bool m_Running = true;
        bool m_Minimized = false;
        bool m_ZeroSize = false;
    private:
        friend int Main(int argc, char** argv);
        Application(ApplicationInfo const& info);
        ~Application();
        void Run();
    };
}
