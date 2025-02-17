#pragma once

namespace eng
{
    struct ApplicationInfo
    {

    };

    class Application
    {
    public:
        static Application& Get();

    private:
        friend int Main(int argc, char** argv);

        Application(const ApplicationInfo& info);
        ~Application();

        void Run();
    private:
        inline static Application* s_Application = nullptr;
    };
}
