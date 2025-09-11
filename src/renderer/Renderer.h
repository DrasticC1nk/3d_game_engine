#pragma once

#define NOMINMAX

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <map>
#include <string>

#include "Mesh.h"

class Scene;
struct GameObject;

struct ConstantBufferData
{
    DirectX::XMMATRIX model, view, projection;
    DirectX::XMFLOAT3 lightPos;

    float shininess;

    DirectX::XMFLOAT3 viewPos;

    float pad1;

    DirectX::XMFLOAT4 lightColor;
};


class Renderer
{
public:

    Renderer();
    ~Renderer();

    bool Init(HWND hWnd);

    void Shutdown();
    void OnResize(int width, int height);

    void BeginFrame();
    void RenderFrame(Scene& scene);
    void EndFrame();

    ID3D11Device* GetDevice() const 
    { 
        return m_device;
    }
    ID3D11DeviceContext* GetContext() const 
    { 
        return m_context; 
    }

private:

    bool CreateDeviceAndSwapChain(HWND hWnd);
    bool CreateMainRenderTarget(int width, int height);
    bool CreateShaders();
    bool CreateDepthStencilStates();
    bool CreateConstantBuffers();
    bool CreateRasterizerStates();
    bool CreateInputLayout();
    bool CreateMeshes();
    void CreateGrid();
    void RenderObject(GameObject& obj, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_rtv = nullptr;
    ID3D11DepthStencilView* m_dsv = nullptr;

    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader* m_lightingPixelShader = nullptr;
    ID3D11PixelShader* m_unlitPixelShader = nullptr;
    ID3D11PixelShader* m_outlinePixelShader = nullptr;

    ID3D11Buffer* m_constantBuffer = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;
    ID3D11RasterizerState* m_rasterState = nullptr;
    ID3D11RasterizerState* m_outlineRasterState = nullptr;
    ID3D11DepthStencilState* m_stencilWriteState = nullptr;
    ID3D11DepthStencilState* m_stencilTestState = nullptr;

    std::map<ShapeType, Mesh> m_meshes;

    ID3D11Buffer* m_gridVertexBuffer = nullptr;
    UINT m_gridVertexCount = 0;
};
