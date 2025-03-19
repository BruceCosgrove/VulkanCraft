#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Input/LayerStack.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include <string>

struct GLFWwindow;

namespace eng
{
    class Window;
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
                m_LayerProducers.push_back([](Window& window) -> std::unique_ptr<Layer> { return std::make_unique<T>(window); });
            }
        private:
            friend class Window;
            std::vector<LayerProducer> m_LayerProducers;
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

        void PushLayer(LayerProducer const& layerProducer);
        void PushLayer(std::unique_ptr<Layer>&& layer);
        std::unique_ptr<Layer> PopLayer();

        void PushOverlay(LayerProducer const& overlayProducer);
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
        detail::LayerStack m_LayerStack;

        f64 m_LastUpdateTime = 0.0;
        f64 m_LastRenderTime = 0.0;
        bool m_Minimized = false;
        bool m_ZeroSize = false;
    };
}
