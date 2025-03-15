#include "Entrypoint.hpp"
#include "Engine/Core/AssertOrVerify.hpp"

namespace eng
{
    static Application* s_Application = nullptr;

    Application& Application::Get()
    {
        ENG_ASSERT(s_Application != nullptr, "Tried to get the application before it exists.");
        return *s_Application;
    }

    static int Main(int argc, char** argv)
    {
        // Deallocate engineInfo once it's no longer needed.
        {
            EngineInfo engineInfo = ProvideEngineInfo(argc, argv);
            Log::Initialize(engineInfo.LogInfo);

            // Create the application.
            ENG_ASSERT(s_Application == nullptr, "Tried to create another application.");
            // Allocate the memory first so s_Application is set before the Application constructor.
            s_Application = (Application*)::operator new(sizeof(Application));
            // Construct the application in the newly allocated memory.
            new(s_Application) Application(engineInfo.ApplicationInfo);
        }

        s_Application->Run();

        // Destroy the application.
        ENG_ASSERT(s_Application != nullptr, "Tried to redestroy the application.");
        delete s_Application;
        s_Application = nullptr;

        Log::Shutdown();

        return 0;
    }
}

// Handle multiple possible entrypoints and forward them all into eng::Main.
#if ENG_CONFIG_DIST
    #if ENG_SYSTEM_WINDOWS
        #include <Windows.h>

        int APIENTRY WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
        {
            static_cast<void>(hInst, hInstPrev, lpCmdLine, nCmdShow);
            return eng::Main(__argc, __argv);
        }
    #elif ENG_SYSTEM_LINUX
        // TODO
    #else
        #error Unsupported system.
    #endif
#else
    int main(int argc, char** argv)
    {
        return eng::Main(argc, argv);
    }
#endif
