#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/LayerStack.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include <string>

struct GLFWwindow;

namespace eng
{
    class WindowMinimizeEvent;
    class WindowFramebufferResizeEvent;

    struct WindowInfo
    {
        std::string Title = "TODO: Window Title";
        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;
        bool Resizable = true;
    };

    class Window
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Window);

        GLFWwindow* GetNativeWindow();
        RenderContext& GetRenderContext();
        LayerStack& GetLayerStack();
    private:
        friend class Application;
        Window(WindowInfo const& info);
        void OnEvent(Event& event);
        void OnUpdate();
        void OnRender();

        // TODO: refactor this
        void GetFramebufferSize(std::uint32_t& width, std::uint32_t& height) const;
    private:
        void OnWindowMinimizeEvent(WindowMinimizeEvent& event);
        void OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event);
    private:
        // Helper to streamline window destruction.
        struct NativeWindow
        {
            NativeWindow(WindowInfo const& info, Window* window);
            ~NativeWindow();

            GLFWwindow* Handle;
        private:
            inline static std::uint32_t s_WindowCount = 0;
        };
    private:
        // IMPORTANT: The order of these members is critical for initialization and shutdown.

        NativeWindow m_NativeWindow;
        RenderContext m_RenderContext;
        LayerStack m_LayerStack;

        double m_LastTime = 0.0;
        bool m_Minimized = false;
        bool m_ZeroSize = false;
    };
}
