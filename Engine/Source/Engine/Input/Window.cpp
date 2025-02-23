#include "Window.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Attributes.hpp"
#include "Engine/Core/FunctionBindings.hpp"
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

    LayerStack& Window::GetLayerStack()
    {
        return m_LayerStack;
    }

    Window::Window(WindowInfo const& info)
        : m_NativeWindow(info, this)
        , m_RenderContext(m_NativeWindow.Handle)
    {
        // Now that the window is fully initialized, show it.
        glfwShowWindow(m_NativeWindow.Handle);
    }

    void Window::OnUpdate()
    {
        // Calculate the time since the last update/frame combo.
        // TODO: separate update/render threads, and only pass the time to the update thread.
        // TODO: think about implementing FixedUpdate.
        double currentTime = glfwGetTime();
        Timestep timestep = static_cast<float>(currentTime - m_LastTime);
        m_LastTime = currentTime;

        m_LayerStack.OnUpdate(timestep);
    }

    void Window::OnRender()
    {
        if (m_Minimized or m_ZeroSize)
            return;

        m_RenderContext.BeginFrame();
        m_LayerStack.OnRender();
        m_RenderContext.EndFrame();
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
            glfwSetErrorCallback([](int error, char const* description)
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

        Handle = glfwCreateWindow((int)info.Width, (int)info.Height, info.Title.c_str(), nullptr, nullptr);
        ENG_ASSERT(Handle != nullptr, "Failed to create window.");
        glfwSetWindowUserPointer(Handle, window);

        // Add all event callbacks.

        glfwSetWindowPosCallback(Handle, [](GLFWwindow* handle, int xpos, int ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMoveEvent event(static_cast<std::int32_t>(xpos), static_cast<std::int32_t>(ypos));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowSizeCallback(Handle, [](GLFWwindow* handle, int width, int height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowResizeEvent event(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
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

        glfwSetWindowFocusCallback(Handle, [](GLFWwindow* handle, int focused)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFocusEvent event(static_cast<bool>(focused));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowIconifyCallback(Handle, [](GLFWwindow* handle, int iconified)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMinimizeEvent event(static_cast<bool>(iconified));
            window.OnWindowMinimizeEvent(event);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowMaximizeCallback(Handle, [](GLFWwindow* handle, int maximized)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMaximizeEvent event(static_cast<bool>(maximized));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetFramebufferSizeCallback(Handle, [](GLFWwindow* handle, int width, int height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFramebufferResizeEvent event(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
            window.OnWindowFramebufferResizeEvent(event);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetWindowContentScaleCallback(Handle, [](GLFWwindow* handle, float xscale, float yscale)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowContentScaleEvent event(xscale, yscale);
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetDropCallback(Handle, [](GLFWwindow* handle, int path_count, char const* paths[])
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowPathDropEvent event({paths, static_cast<std::size_t>(path_count)});
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetMouseButtonCallback(Handle, [](GLFWwindow* handle, int button, int action, int mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressEvent event(static_cast<MouseButton>(button), static_cast<Modifiers>(mods));
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleaseEvent event(static_cast<MouseButton>(button), static_cast<Modifiers>(mods));
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
            }
        });

        glfwSetCursorPosCallback(Handle, [](GLFWwindow* handle, double xpos, double ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseMoveEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetCursorEnterCallback(Handle, [](GLFWwindow* handle, int entered)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseEnterEvent event(static_cast<bool>(entered));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetScrollCallback(Handle, [](GLFWwindow* handle, double xoffset, double yoffset)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseScrollEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
            window.m_LayerStack.OnEvent(event);
        });

        glfwSetKeyCallback(Handle, [](GLFWwindow* handle, int key, int scancode, int action, int mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressEvent event(static_cast<Keycode>(key), static_cast<Modifiers>(mods));
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleaseEvent event(static_cast<Keycode>(key), static_cast<Modifiers>(mods));
                    window.m_LayerStack.OnEvent(event);
                    break;
                }
                // Not using repeat events.
                case GLFW_REPEAT: break;
            }
        });

        glfwSetCharCallback(Handle, [](GLFWwindow* handle, unsigned int codepoint)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            KeyCharTypeEvent event(static_cast<std::uint32_t>(codepoint));
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
