#pragma once

#define NOMINMAX

#include <windows.h>
#include <memory>
#include <vector>

class Window;
class Renderer;
class Scene;
class UIManager;

class Application
{
public:

    Application();
    ~Application();

    bool Init(HINSTANCE hInstance);

    void Run();
    void Shutdown();

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

    void Update();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<UIManager> m_uiManager;

    bool m_keys[256] = { false };
    bool m_isRunning = true;
    bool m_isMouseCaptured = false;
};
