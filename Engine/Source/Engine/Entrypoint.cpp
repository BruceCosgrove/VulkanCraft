#include "Entrypoint.hpp"

namespace eng
{
    static int Main(int argc, char** argv)
    {
        EngineInfo engineInfo = GetEngineInfo(argc, argv);
        Log::Initialize(engineInfo.LogInfo);

        Application* application = new Application(engineInfo.ApplicationInfo);
        application->Run();
        delete application;

        Log::Shutdown();

        return 0;
    }
}

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
