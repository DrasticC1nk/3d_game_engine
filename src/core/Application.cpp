#include "Application.h"
#include "Window.h"
#include "../Renderer/Renderer.h"
#include "../Scene/Scene.h"
#include "../UI/UIManager.h"
#include "../Scene/Camera.h" 

Application::Application() {}
Application::~Application() {}

bool Application::Init(HINSTANCE hInstance)
{
    m_window = std::make_unique<Window>();

    if (!m_window->Create(this, hInstance, L"DX11 Scene Editor", 1280, 720))
    {
        return false;
    }

    m_renderer = std::make_unique<Renderer>();

    if (!m_renderer->Init(m_window->GetHWND()))
    {
        return false;
    }

    m_scene = std::make_unique<Scene>();
    m_uiManager = std::make_unique<UIManager>();

    if (!m_uiManager->Init(m_window->GetHWND(), m_renderer->GetDevice(), m_renderer->GetContext()))
    {
        return false;
    }

    m_window->Show();

    return true;
}

void Application::Run()
{
    MSG msg = {};

    while (m_isRunning)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            DispatchMessage(&msg);
        }
        else
        {
            Update();

            m_renderer->BeginFrame();

            m_uiManager->BeginFrame();
            m_uiManager->RenderUI(*m_scene); 

            m_renderer->RenderFrame(*m_scene);

            m_uiManager->EndFrame();
            m_renderer->EndFrame();
        }
    }
}

void Application::Shutdown()
{
    m_uiManager->Shutdown();
    m_renderer->Shutdown();
}

void Application::Update()
{
    m_scene->GetCamera().Update(m_keys);

    for (auto& obj : m_scene->GetGameObjects())
    {
        if (obj.enableAutoRotation)
        {
            obj.autoRotationAngle += 0.005f;
        }
    }
}

LRESULT Application::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:

            m_isRunning = false;

            PostQuitMessage(0);

            return 0;

        case WM_SIZE:

            if (m_renderer && wParam != SIZE_MINIMIZED)
            {
                m_renderer->OnResize(LOWORD(lParam), HIWORD(lParam));
            }

            return 0;

        case WM_KEYDOWN:

            if (wParam < 256)
            {
                m_keys[wParam] = true;
            }

            return 0;

        case WM_KEYUP:

            if (wParam < 256)
            {
                m_keys[wParam] = false;
            }

            return 0;

        case WM_LBUTTONDOWN:

            if (m_uiManager->WantCaptureMouse())
            {
                return 0;
            }

            m_scene->SelectObject(hwnd, LOWORD(lParam), HIWORD(lParam));

            return 0;

        case WM_RBUTTONDOWN:
        {
            if (m_uiManager->WantCaptureMouse())
            {
                return 0;
            }

            m_isMouseCaptured = true;

            SetCapture(hwnd);
            ShowCursor(FALSE);

            RECT rect;
            GetClientRect(hwnd, &rect);
            POINT center = { rect.right / 2, rect.bottom / 2 };
            ClientToScreen(hwnd, &center);

            SetCursorPos(center.x, center.y);

            return 0;
        }

        case WM_RBUTTONUP:
        {
            m_isMouseCaptured = false;

            ReleaseCapture();

            ShowCursor(TRUE);

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (m_isMouseCaptured)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                POINT center = { rect.right / 2, rect.bottom / 2 };
                ClientToScreen(hwnd, &center);

                POINT currentMousePos;
                GetCursorPos(&currentMousePos);

                float dx = static_cast<float>(currentMousePos.x - center.x);
                float dy = static_cast<float>(currentMousePos.y - center.y);

                if (dx != 0 || dy != 0)
                {
                    m_scene->GetCamera().UpdateMouse(dx, dy);

                    SetCursorPos(center.x, center.y);
                }
            }
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}