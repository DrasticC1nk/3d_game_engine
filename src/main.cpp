#define NOMINMAX

#include <windows.h>
#include "Core/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    Application app;

    if (app.Init(hInstance))
    {
        app.Run();
    }

    app.Shutdown();

    return 0;
}