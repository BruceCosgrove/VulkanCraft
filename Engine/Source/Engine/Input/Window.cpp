#include "Window.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Attributes.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/Input/Event/MouseEvents.hpp"
#include "Engine/Input/Event/KeyEvents.hpp"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

namespace eng
{
    static std::uint32_t s_WindowCount = 0;

    Window::Window(WindowInfo const& info, std::function<void(Event&)>&& eventCallback)
        : m_EventCallback(std::move(eventCallback))
    {
        // If this is the first window created, initialize GLFW.
        if (s_WindowCount++ == 0)
        {
            ENG_VERIFY(glfwInit(), "Failed to initialize GLFW.");

            glfwSetErrorCallback([](int error, char const* description)
            {
                ENG_LOG_ERROR("GLFW Error ({}): {}", error, description);
            });
        }

        glfwWindowHint(GLFW_RESIZABLE, info.Resizable);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, true);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);
        glfwWindowHint(GLFW_MAXIMIZED, false);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Vulkan
        glfwWindowHint(GLFW_VISIBLE, false); // Make the window invisible while still initializing it.

        m_Window = glfwCreateWindow((int)info.Width, (int)info.Height, info.Title.c_str(), nullptr, nullptr);
        ENG_ASSERT(m_Window != nullptr, "Failed to create window.");
        glfwSetWindowUserPointer(m_Window, this);

        // Add all event callbacks.

        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* handle, int xpos, int ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMoveEvent event(static_cast<std::int32_t>(xpos), static_cast<std::int32_t>(ypos));
            window.m_EventCallback(event);
        });

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* handle, int width, int height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowResizeEvent event(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
            window.m_EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* handle)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowCloseEvent event;
            window.m_EventCallback(event);
        });

        glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow* handle)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowRefreshEvent event;
            window.m_EventCallback(event);
        });

        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* handle, int focused)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFocusEvent event(static_cast<bool>(focused));
            window.m_EventCallback(event);
        });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* handle, int iconified)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMinimizeEvent event(static_cast<bool>(iconified));
            window.m_EventCallback(event);
        });

        glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* handle, int maximized)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowMaximizeEvent event(static_cast<bool>(maximized));
            window.m_EventCallback(event);
        });

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* handle, int width, int height)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowFramebufferResizeEvent event(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
            window.m_EventCallback(event);
        });

        glfwSetWindowContentScaleCallback(m_Window, [](GLFWwindow* handle, float xscale, float yscale)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowContentScaleEvent event(xscale, yscale);
            window.m_EventCallback(event);
        });

        glfwSetDropCallback(m_Window, [](GLFWwindow* handle, int path_count, char const* paths[])
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            WindowPathDropEvent event({paths, static_cast<std::size_t>(path_count)});
            window.m_EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* handle, int button, int action, int mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressEvent event(static_cast<MouseButton>(button), static_cast<Modifiers>(mods));
                    window.m_EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleaseEvent event(static_cast<MouseButton>(button), static_cast<Modifiers>(mods));
                    window.m_EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* handle, double xpos, double ypos)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseMoveEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            window.m_EventCallback(event);
        });

        glfwSetCursorEnterCallback(m_Window, [](GLFWwindow* handle, int entered)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseEnterEvent event(static_cast<bool>(entered));
            window.m_EventCallback(event);
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* handle, double xoffset, double yoffset)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            MouseScrollEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
            window.m_EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* handle, int key, int scancode, int action, int mods)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressEvent event(static_cast<Keycode>(key), static_cast<Modifiers>(mods));
                    window.m_EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleaseEvent event(static_cast<Keycode>(key), static_cast<Modifiers>(mods));
                    window.m_EventCallback(event);
                    break;
                }
                // Not using repeat events.
                case GLFW_REPEAT: break;
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* handle, unsigned int codepoint)
        {
            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            KeyCharTypeEvent event(static_cast<std::uint32_t>(codepoint));
            window.m_EventCallback(event);
        });

        // Now that the window is fully initialized, show it.
        glfwShowWindow(m_Window);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);

        // If this is the last window destroyed, terminate GLFW.
        if (--s_WindowCount == 0)
            glfwTerminate();
    }
}
