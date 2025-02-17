#include "Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"

namespace eng
{
    Application& Application::Get()
    {
        ENG_ASSERT(s_Application != nullptr);
        return *s_Application;
    }

    Application::Application(const ApplicationInfo& info)
    {
        ENG_ASSERT(s_Application == nullptr);
        s_Application = this;
    }

    Application::~Application()
    {
        ENG_ASSERT(s_Application != nullptr);
        s_Application = nullptr;
    }

    void Application::Run()
    {

    }
}
