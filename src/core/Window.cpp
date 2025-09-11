#include "Window.h"
#include "Application.h"
#include "../../external/imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window() {}

Window::~Window()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}

bool Window::Create(Application* app, HINSTANCE hInstance, const wchar_t* title, int width, int height)
{
    m_app = app;

    WNDCLASS wc = {};

    wc.lpfnWndProc = Window::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DX11WindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    m_hWnd = CreateWindow(L"DX11WindowClass", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hInstance, this );

    return m_hWnd != nullptr;
}

void Window::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true;
    }

    Window* window = nullptr;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;

        window = (Window*)pCreate->lpCreateParams;

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else
    {
        window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (window && window->m_app)
    {
        return window->m_app->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}