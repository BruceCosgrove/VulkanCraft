#include "Window.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/Input/Event/MouseEvents.hpp"
#include "Engine/Input/Event/KeyEvents.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

namespace eng
{
    GLFWwindow* Window::GetNativeWindow()
    {
        return m_NativeWindow.Handle;
    }

    RenderContext& Window::GetRenderContext()
    {
        return m_RenderContext;
    }

    void Window::PushLayer(std::unique_ptr<Layer>&& layer)
    {
        m_LayerStack.PushLayer(std::move(layer), this);
    }

    std::unique_ptr<Layer> Window::PopLayer()
    {
        return m_LayerStack.PopLayer();
    }

    void Window::PushOverlay(std::unique_ptr<Layer>&& overlay)
    {
        m_LayerStack.PushOverlay(std::move(overlay), this);
    }

    std::unique_ptr<Layer> Window::PopOverlay()
    {
        return m_LayerStack.PopOverlay();
    }

    Window::Window(WindowInfo const& info)
        : m_NativeWindow(info, this)
        , m_RenderContext(m_NativeWindow.Handle)
        , m_LayerStack(info.Layers.m_LayersProducers, this)
    {
        // Now that the window is fully initialized, show it.
        glfwShowWindow(m_NativeWindow.Handle);
    }

    Window::~Window()
    {
        VkResult result = vkDeviceWaitIdle(m_RenderContext.GetDevice());
        ENG_ASSERT(result == VK_SUCCESS, "Failed to wait for the device to stop working. This really shouldn't happen.");
    }

    void Window::OnEvent(Event& event)
    {
        m_LayerStack.OnEvent(event);
    }

    void Window::OnUpdate()
    {
        // Calculate the time since the last update/frame combo.
        // TODO: separate update/render threads, and only pass the time to the update thread.
        // TODO: think about implementing FixedUpdate.

        f64 currentTime = glfwGetTime();
        Timestep timestep = static_cast<f32>(currentTime - m_LastTime);
        m_LastTime = currentTime;

        m_LayerStack.OnUpdate(timestep);
    }

    void Window::OnRender()
    {
        if (m_Minimized or m_ZeroSize)
            return;

        if (m_RenderContext.BeginFrame())
        {
            m_LayerStack.OnRender();
            m_RenderContext.EndFrame();
        }
    }

    void Window::GetFramebufferSize(u32& width, u32& height) const
    {
        glfwGetFramebufferSize(m_NativeWindow.Handle, (i32*)&width, (i32*)&height);
    }

    void Window::OnWindowMinimizeEvent(WindowMinimizeEvent& event)
    {
        m_Minimized = event.IsMinimized();
    }

    void Window::OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event)
    {
        m_ZeroSize = event.GetFramebufferWidth() == 0 or event.GetFramebufferHeight() == 0;

        // Block zero-size events from propagating to the Client.
        if (m_ZeroSize)
            event.Handle();
    }

    Window::NativeWindow::NativeWindow(WindowInfo const& info, Window* window)
    {
        // If this is the first window created, initialize GLFW.
        if (s_WindowCount++ == 0)
        {
            glfwSetErrorCallback([](i32 error, char const* description)
            {
                ENG_LOG_ERROR("GLFW Error ({}): {}", error, description);
            });

            ENG_VERIFY(glfwInit(), "Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_RESIZABLE, info.Resizable);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, true);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);
        glfwWindowHint(GLFW_MAXIMIZED, false);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Vulkan
        glfwWindowHint(GLFW_VISIBLE, false); // Make the window invisible while still initializing it.

        Handle = glfwCreateWindow(static_cast<i32>(info.Width), static_cast<i32>(info.Height), info.Title.c_str(), nullptr, nullptr);
        ENG_ASSERT(Handle != nullptr, "Failed to create window.");

        // Set the user pointer to this window to use in event callbacks.
        glfwSetWindowUserPointer(Handle, window);

        // Add all event callbacks.

        glfwSetWindowPosCallback(Handle, [](GLFWwindow* handle, i32 xpos, i32 ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMoveEvent event(static_cast<i32>(xpos), static_cast<i32>(ypos));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowSizeCallback(Handle, [](GLFWwindow* handle, i32 width, i32 height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowCloseCallback(Handle, [](GLFWwindow* handle)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowCloseEvent event;
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowRefreshCallback(Handle, [](GLFWwindow* handle)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowRefreshEvent event;
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowFocusCallback(Handle, [](GLFWwindow* handle, i32 focused)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFocusEvent event(static_cast<bool>(focused));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowIconifyCallback(Handle, [](GLFWwindow* handle, i32 iconified)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMinimizeEvent event(static_cast<bool>(iconified));
            window.OnWindowMinimizeEvent(event);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowMaximizeCallback(Handle, [](GLFWwindow* handle, i32 maximized)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMaximizeEvent event(static_cast<bool>(maximized));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetFramebufferSizeCallback(Handle, [](GLFWwindow* handle, i32 width, i32 height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFramebufferResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
            window.OnWindowFramebufferResizeEvent(event);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowContentScaleCallback(Handle, [](GLFWwindow* handle, f32 xscale, f32 yscale)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowContentScaleEvent event(xscale, yscale);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetDropCallback(Handle, [](GLFWwindow* handle, i32 path_count, char const* paths[])
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowPathDropEvent event({paths, static_cast<u64>(path_count)});
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetMouseButtonCallback(Handle, [](GLFWwindow* handle, i32 button, i32 action, i32 mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                case GLFW_RELEASE:
                {
                    MouseButtonPressEvent event(static_cast<MouseButton>(button), static_cast<Modifiers>(mods), action == GLFW_PRESS);
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
            }
        });

        glfwSetCursorPosCallback(Handle, [](GLFWwindow* handle, f64 xpos, f64 ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseMoveEvent event(static_cast<f32>(xpos), static_cast<f32>(ypos));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetCursorEnterCallback(Handle, [](GLFWwindow* handle, i32 entered)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseEnterEvent event(static_cast<bool>(entered));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetScrollCallback(Handle, [](GLFWwindow* handle, f64 xoffset, f64 yoffset)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseScrollEvent event(static_cast<f32>(xoffset), static_cast<f32>(yoffset));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetKeyCallback(Handle, [](GLFWwindow* handle, i32 key, i32 scancode, i32 action, i32 mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                case GLFW_RELEASE:
                {
                    KeyPressEvent event(static_cast<Keycode>(key), static_cast<Modifiers>(mods), action == GLFW_PRESS);
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
                // Not using repeat events.
                case GLFW_REPEAT: break;
            }
        });

        glfwSetCharCallback(Handle, [](GLFWwindow* handle, u32 codepoint)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            KeyCharTypeEvent event(static_cast<u32>(codepoint));
            window.m_LayerStack.OnEvent(event);
        });
    }

    Window::NativeWindow::~NativeWindow()
    {
        // Shutdown the window.
        glfwDestroyWindow(Handle);
        // If this is the last window destroyed, also shutdown GLFW.
        if (--s_WindowCount == 0)
            glfwTerminate();
    }
}
