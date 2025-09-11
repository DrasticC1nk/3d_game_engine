#pragma once

#include <d3d11.h>
#include <string>

class Scene; 

class UIManager
{
public:

    UIManager();
    ~UIManager();

    bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

    void Shutdown();

    void BeginFrame();
    void RenderUI(Scene& scene);
    void EndFrame();

    bool WantCaptureMouse() const;
    bool WantCaptureKeyboard() const;

private:

    void ShowMenuBar(Scene& scene);
    void ShowSceneControls(Scene& scene);
    void ShowObjectProperties(Scene& scene);

    void SaveScene(Scene& scene);
    void OpenScene(Scene& scene);

    void WriteSceneAndUILayoutToFile(const std::string& filepath, Scene& scene);
    void LoadSceneAndUILayoutFromFile(const std::string& filepath, Scene& scene);
};
