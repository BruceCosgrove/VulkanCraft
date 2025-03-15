#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/LayerStack.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include <string>

struct GLFWwindow;

namespace eng
{
    class WindowMinimizeEvent;
    class WindowFramebufferResizeEvent;

    // Simplify Client layer adding syntax.
    namespace detail
    {
        class WindowLayersInfo
        {
        public:
            template <class T>
            requires(std::is_base_of_v<Layer, T>)
            void Add()
            {
                m_LayersProducers.push_back([]() -> std::unique_ptr<Layer> { return std::make_unique<T>(); });
            }
        private:
            friend class Window;
            std::vector<std::unique_ptr<Layer>(*)()> m_LayersProducers;
        };
    }

    struct WindowInfo
    {
        string Title = "TODO: Window Title";
        u32 Width = 1280;
        u32 Height = 720;
        bool Resizable = true;
        detail::WindowLayersInfo Layers;
    };

    class Window
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(Window);

        GLFWwindow* GetNativeWindow();
        RenderContext& GetRenderContext();

        void PushLayer(std::unique_ptr<Layer>&& layer);
        std::unique_ptr<Layer> PopLayer();

        void PushOverlay(std::unique_ptr<Layer>&& overlay);
        std::unique_ptr<Layer> PopOverlay();
    private:
        friend class Application;

        Window(WindowInfo const& info);
        ~Window();

        void OnEvent(Event& event);
        void OnUpdate();
        void OnRender();

        // TODO: refactor this
        void GetFramebufferSize(u32& width, u32& height) const;
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
            inline static u32 s_WindowCount = 0;
        };
    private:
        // IMPORTANT: The order of these members is critical for initialization and shutdown.

        NativeWindow m_NativeWindow;
        RenderContext m_RenderContext;
        LayerStack m_LayerStack;

        f64 m_LastTime = 0.0;
        bool m_Minimized = false;
        bool m_ZeroSize = false;
    };
}
