#pragma once

#define NOMINMAX

#include <windows.h>

class Application; 

class Window
{
public:

    Window();
    ~Window();

    bool Create(Application* app, HINSTANCE hInstance, const wchar_t* title, int width, int height);

    void Show();

    HWND GetHWND() const 
    { 
        return m_hWnd; 
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

    HWND m_hWnd = nullptr;

    Application* m_app = nullptr;
};