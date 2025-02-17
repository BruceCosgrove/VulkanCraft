#include "Entrypoint.hpp"

#if ENG_SYSTEM_WINDOWS

namespace eng
{
    int Main(int argc, char** argv)
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

#include <Windows.h>

int APIENTRY WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    static_cast<void>(hInst, hInstPrev, lpCmdLine, nCmdShow);
    return eng::Main(__argc, __argv);
}

#else // !ENG_CONFIG_DIST

int main(int argc, char** argv)
{
    return eng::Main(argc, argv);
}

#endif // ENG_CONFIG_DIST

#endif // ENG_SYSTEM_WINDOWS
