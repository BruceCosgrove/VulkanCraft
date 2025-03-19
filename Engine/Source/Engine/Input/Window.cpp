#include "Window.hpp"
#include "Engine/Core/Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/Input/Event/MouseEvents.hpp"
#include "Engine/Input/Event/KeyEvents.hpp"
#include <glfw/glfw3.h>
#include <array>

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

    void Window::PushLayer(LayerProducer const& layerProducer)
    {
        m_LayerStack.PushLayer(layerProducer, *this);
    }

    void Window::PushLayer(std::unique_ptr<Layer>&& layer)
    {
        m_LayerStack.PushLayer(std::move(layer));
    }

    std::unique_ptr<Layer> Window::PopLayer()
    {
        return m_LayerStack.PopLayer();
    }

    void Window::PushOverlay(LayerProducer const& layerProducer)
    {
        m_LayerStack.PushOverlay(layerProducer, *this);
    }

    void Window::PushOverlay(std::unique_ptr<Layer>&& overlay)
    {
        m_LayerStack.PushOverlay(std::move(overlay));
    }

    std::unique_ptr<Layer> Window::PopOverlay()
    {
        return m_LayerStack.PopOverlay();
    }

    Window::Window(WindowInfo const& info)
        : m_NativeWindow(info, this)
        , m_RenderContext(m_NativeWindow.Handle)
        , m_LayerStack(info.Layers.m_LayerProducers, *this)
    {
        // Now that the window and Client are fully initialized, show the window.
        glfwShowWindow(m_NativeWindow.Handle);

        m_LastUpdateTime = m_LastRenderTime = glfwGetTime();
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
        f64 currentTime = glfwGetTime();
        Timestep timestep = static_cast<f32>(currentTime - m_LastUpdateTime);
        m_LastUpdateTime = currentTime;

        m_LayerStack.OnUpdate(timestep);
    }

    void Window::OnRender()
    {
        if (m_Minimized or m_ZeroSize)
            return;

        if (m_RenderContext.BeginFrame())
        {
            f64 currentTime = glfwGetTime();
            Timestep timestep = static_cast<f32>(currentTime - m_LastRenderTime);
            m_LastRenderTime = currentTime;

            // TODO: SetWindowLongW, called from ImGui_ImplGlfw_NewFrame,
            // blocks execution if the application is terminated between
            // RenderContext::BeginFrame() and RenderContext::EndFrame().
            if (Application::Get().IsRunning())
                m_LayerStack.OnRender(timestep);
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
                    MouseButtonPressEvent event(static_cast<MouseButton>(button + 1), static_cast<Modifiers>(mods), action == GLFW_PRESS);
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
            if (key == GLFW_KEY_UNKNOWN)
                return;

            Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

            static constexpr auto s_KeyConversions = std::to_array<Keycode>({
                0,0,0,0,0,0,0,0,0,0, // 0-9
                0,0,0,0,0,0,0,0,0,0, // 10-19
                0,0,0,0,0,0,0,0,0,0, // 20-29
                0,0, // 30-31
                /* Printable keys */
                Keycode::Space,          // 32
                0,0,0,0,0,0, // 33-38
                Keycode::Apostrophe,     // 39 /* ' */
                0,0,0,0, // 40-43
                Keycode::Comma,          // 44 /* , */
                Keycode::Minus,          // 45 /* - */
                Keycode::Period,         // 46 /* . */
                Keycode::Slash,          // 47 /* / */
                Keycode::D0,             // 48 /* 0 */
                Keycode::D1,             // 49 /* 1 */
                Keycode::D2,             // 50 /* 2 */
                Keycode::D3,             // 51 /* 3 */
                Keycode::D4,             // 52 /* 4 */
                Keycode::D5,             // 53 /* 5 */
                Keycode::D6,             // 54 /* 6 */
                Keycode::D7,             // 55 /* 7 */
                Keycode::D8,             // 56 /* 8 */
                Keycode::D9,             // 57 /* 9 */
                0, // 58
                Keycode::Semicolon,      // 59 /* ; */
                0, // 60
                Keycode::Equal,          // 61 /* = */
                0,0,0, // 62-64
                Keycode::A,              // 65
                Keycode::B,              // 66
                Keycode::C,              // 67
                Keycode::D,              // 68
                Keycode::E,              // 69
                Keycode::F,              // 70
                Keycode::G,              // 71
                Keycode::H,              // 72
                Keycode::I,              // 73
                Keycode::J,              // 74
                Keycode::K,              // 75
                Keycode::L,              // 76
                Keycode::M,              // 77
                Keycode::N,              // 78
                Keycode::O,              // 79
                Keycode::P,              // 80
                Keycode::Q,              // 81
                Keycode::R,              // 82
                Keycode::S,              // 83
                Keycode::T,              // 84
                Keycode::U,              // 85
                Keycode::V,              // 86
                Keycode::W,              // 87
                Keycode::X,              // 88
                Keycode::Y,              // 89
                Keycode::Z,              // 90
                Keycode::LeftBracket,    // 91 /* [ */
                Keycode::BackSlash,      // 92 /* \ */
                Keycode::RightBracket,   // 93 /* ] */
                0,0, // 94-95
                Keycode::GraveAccent,    // 96 /* ` */
                0,0,0,0, // 97-100
                0,0,0,0,0,0,0,0,0,0, // 101-110
                0,0,0,0,0,0,0,0,0,0, // 111-120
                0,0,0,0,0,0,0,0,0,0, // 121-130
                0,0,0,0,0,0,0,0,0,0, // 131-140
                0,0,0,0,0,0,0,0,0,0, // 141-150
                0,0,0,0,0,0,0,0,0,0, // 151-160
                Keycode::World1,         // 161 /* non-US #1 */
                Keycode::World2,         // 162 /* non-US #2 */
                0,0,0, // 163-165
                0,0,0,0,0,0,0,0,0,0, // 166-175
                0,0,0,0,0,0,0,0,0,0, // 176-185
                0,0,0,0,0,0,0,0,0,0, // 186-195
                0,0,0,0,0,0,0,0,0,0, // 196-205
                0,0,0,0,0,0,0,0,0,0, // 206-215
                0,0,0,0,0,0,0,0,0,0, // 216-225
                0,0,0,0,0,0,0,0,0,0, // 226-235
                0,0,0,0,0,0,0,0,0,0, // 236-245
                0,0,0,0,0,0,0,0,0,0, // 246-255
                /* Function keys */
                Keycode::Escape,         // 256
                Keycode::Enter,          // 257
                Keycode::Tab,            // 258
                Keycode::Backspace,      // 259
                Keycode::Insert,         // 260
                Keycode::Delete,         // 261
                Keycode::Right,          // 262
                Keycode::Left,           // 263
                Keycode::Down,           // 264
                Keycode::Up,             // 265
                Keycode::PageUp,         // 266
                Keycode::PageDown,       // 267
                Keycode::Home,           // 268
                Keycode::End,            // 269
                0,0,0,0,0,0,0,0,0,0, // 270-279
                Keycode::CapsLock,       // 280
                Keycode::ScrollLock,     // 281
                Keycode::NumLock,        // 282
                Keycode::PrintScreen,    // 283
                Keycode::Pause,          // 284
                0,0,0,0,0, // 285-289
                Keycode::F1,             // 290
                Keycode::F2,             // 291
                Keycode::F3,             // 292
                Keycode::F4,             // 293
                Keycode::F5,             // 294
                Keycode::F6,             // 295
                Keycode::F7,             // 296
                Keycode::F8,             // 297
                Keycode::F9,             // 298
                Keycode::F10,            // 299
                Keycode::F11,            // 300
                Keycode::F12,            // 301
                Keycode::F13,            // 302
                Keycode::F14,            // 303
                Keycode::F15,            // 304
                Keycode::F16,            // 305
                Keycode::F17,            // 306
                Keycode::F18,            // 307
                Keycode::F19,            // 308
                Keycode::F20,            // 309
                Keycode::F21,            // 310
                Keycode::F22,            // 311
                Keycode::F23,            // 312
                Keycode::F24,            // 313
                Keycode::F25,            // 314
                0,0,0,0,0, // 315-319
                /* Keypad */
                Keycode::KeyPad0,        // 320
                Keycode::KeyPad1,        // 321
                Keycode::KeyPad2,        // 322
                Keycode::KeyPad3,        // 323
                Keycode::KeyPad4,        // 324
                Keycode::KeyPad5,        // 325
                Keycode::KeyPad6,        // 326
                Keycode::KeyPad7,        // 327
                Keycode::KeyPad8,        // 328
                Keycode::KeyPad9,        // 329
                Keycode::KeyPadDecimal,  // 330
                Keycode::KeyPadDivide,   // 331
                Keycode::KeyPadMultiply, // 332
                Keycode::KeyPadSubtract, // 333
                Keycode::KeyPadAdd,      // 334
                Keycode::KeyPadEnter,    // 335
                Keycode::KeyPadEqual,    // 336
                0,0,0, // 337-339
                Keycode::LeftShift,      // 340
                Keycode::LeftControl,    // 341
                Keycode::LeftAlt,        // 342
                Keycode::LeftSuper,      // 343
                Keycode::RightShift,     // 344
                Keycode::RightControl,   // 345
                Keycode::RightAlt,       // 346
                Keycode::RightSuper,     // 347
                Keycode::Menu,           // 348
            });

            switch (action)
            {
                case GLFW_PRESS:
                case GLFW_RELEASE:
                {
                    KeyPressEvent event(s_KeyConversions.at(key), static_cast<Modifiers>(mods), action == GLFW_PRESS);
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

            KeyTypeEvent event(static_cast<u32>(codepoint));
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
