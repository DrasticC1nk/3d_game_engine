#include "Renderer.h"
#include "../Scene/Scene.h"
#include <d3dcompiler.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

static std::vector<byte> ReadShaderFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) 
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<byte> buffer(fileSize);

    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    return buffer;
}


Renderer::Renderer() {}
Renderer::~Renderer() {}

bool Renderer::Init(HWND hWnd)
{
    RECT rc;

    GetClientRect(hWnd, &rc);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    if (!CreateDeviceAndSwapChain(hWnd))
    {
        return false;
    }
    if (!CreateMainRenderTarget(width, height))
    {
        return false;
    }
    if (!CreateShaders())
    {
        return false;
    }
    if (!CreateConstantBuffers())
    {
        return false;
    }
    if (!CreateRasterizerStates())
    {
        return false;
    }
    if (!CreateDepthStencilStates())
    {
        return false;
    }
    if (!CreateInputLayout())
    {
        return false;
    }
    if (!CreateMeshes())
    {
        return false;
    }

    CreateGrid();

    m_context->RSSetState(m_rasterState);

    return true;
}

void Renderer::Shutdown()
{
    if (m_device)
    {
        m_device->Release();
    }
    if (m_context)
    {
        m_context->Release();
    }
    if (m_swapChain)
    {
        m_swapChain->Release();
    }
    if (m_rtv)
    {
        m_rtv->Release();
    }
    if (m_dsv)
    {
        m_dsv->Release();
    }
    if (m_vertexShader)
    {
        m_vertexShader->Release();
    }
    if (m_lightingPixelShader)
    {
        m_lightingPixelShader->Release();
    }
    if (m_unlitPixelShader)
    {
        m_unlitPixelShader->Release();
    }
    if (m_outlinePixelShader)
    {
        m_outlinePixelShader->Release();
    }
    if (m_constantBuffer)
    {
        m_constantBuffer->Release();
    }
    if (m_inputLayout)
    {
        m_inputLayout->Release();
    }
    if (m_rasterState)
    {
        m_rasterState->Release();
    }
    if (m_outlineRasterState)
    {
        m_outlineRasterState->Release();
    }
    if (m_stencilWriteState)
    {
        m_stencilWriteState->Release();
    }
    if (m_stencilTestState)
    {
        m_stencilTestState->Release();
    }
    if (m_gridVertexBuffer)
    {
        m_gridVertexBuffer->Release();
    }
}


void Renderer::OnResize(int width, int height)
{
    if (!m_swapChain)
    {
        return;
    }

    if (m_rtv)
    {
        m_rtv->Release();
    }
    if (m_dsv)
    {
        m_dsv->Release();
    }

    m_context->OMSetRenderTargets(0, 0, 0);

    m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    CreateMainRenderTarget(width, height);
}


void Renderer::BeginFrame()
{
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    m_context->ClearRenderTargetView(m_rtv, clearColor);
    m_context->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}


void Renderer::RenderFrame(Scene& scene)
{
    Camera& camera = scene.GetCamera();

    DirectX::XMMATRIX view = camera.GetViewMatrix();
    DirectX::XMMATRIX proj = camera.GetProjectionMatrix();

    m_context->IASetInputLayout(m_inputLayout);
    m_context->VSSetShader(m_vertexShader, 0, 0);
    m_context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
    m_context->PSSetConstantBuffers(0, 1, &m_constantBuffer);

    for (int i = 0; i < scene.GetGameObjects().size(); ++i)
    {
        GameObject& obj = scene.GetGameObjects()[i];
        
        bool isSelected = (i == scene.GetSelectedObjectIndex());

        if (isSelected)
        {
            UINT stencilRef = 1;

            m_context->OMSetDepthStencilState(m_stencilWriteState, stencilRef);
            m_context->RSSetState(m_rasterState);
            m_context->PSSetShader(obj.applyLighting ? m_lightingPixelShader : m_unlitPixelShader, 0, 0);

            RenderObject(obj, view, proj); 

            m_context->OMSetDepthStencilState(m_stencilTestState, stencilRef);
            m_context->RSSetState(m_outlineRasterState);
            m_context->PSSetShader(m_outlinePixelShader, 0, 0);

            DirectX::XMMATRIX outlineModel = obj.GetTransformMatrix() * DirectX::XMMatrixScaling(1.08f, 1.08f, 1.08f);

            ConstantBufferData cb;

            cb.model = DirectX::XMMatrixTranspose(outlineModel);
            cb.view = DirectX::XMMatrixTranspose(view);
            cb.projection = DirectX::XMMatrixTranspose(proj);

            m_context->UpdateSubresource(m_constantBuffer, 0, 0, &cb, 0, 0);

            Mesh& mesh = m_meshes[obj.type];
            UINT stride = sizeof(Vertex), offset = 0;

            m_context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
            m_context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_context->DrawIndexed(mesh.indexCount, 0, 0);

            m_context->OMSetDepthStencilState(nullptr, 0);
        }
        else
        {
            m_context->RSSetState(m_rasterState);
            m_context->PSSetShader(obj.applyLighting ? m_lightingPixelShader : m_unlitPixelShader, 0, 0);

            RenderObject(obj, view, proj);
        }
    }

    if (m_gridVertexBuffer)
    {
        ConstantBufferData cb = {}; 

        cb.model = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
        cb.view = DirectX::XMMatrixTranspose(view);
        cb.projection = DirectX::XMMatrixTranspose(proj);

        m_context->UpdateSubresource(m_constantBuffer, 0, 0, &cb, 0, 0);
        m_context->PSSetShader(m_unlitPixelShader, 0, 0);

        UINT stride = sizeof(Vertex), offset = 0;

        m_context->IASetVertexBuffers(0, 1, &m_gridVertexBuffer, &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_context->Draw(m_gridVertexCount, 0);
    }
}


void Renderer::RenderObject(GameObject& obj, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    ConstantBufferData cb = {};

    cb.model = DirectX::XMMatrixTranspose(obj.GetTransformMatrix());
    cb.view = DirectX::XMMatrixTranspose(view);
    cb.projection = DirectX::XMMatrixTranspose(proj);

    DirectX::XMStoreFloat3(&cb.viewPos, DirectX::XMVectorSet(0, 0, 0, 0)); 

    cb.lightPos = DirectX::XMFLOAT3(2, 10, -5);
    cb.lightColor = DirectX::XMFLOAT4(1, 1, 1, 1);
    cb.shininess = 128.0f;

    m_context->UpdateSubresource(m_constantBuffer, 0, 0, &cb, 0, 0);

    Mesh& mesh = m_meshes[obj.type];

    UINT stride = sizeof(Vertex), offset = 0;

    m_context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->DrawIndexed(mesh.indexCount, 0, 0);
}

void Renderer::EndFrame()
{
    m_swapChain->Present(1, 0);
}

bool Renderer::CreateDeviceAndSwapChain(HWND hWnd)
{
    RECT rc;

    GetClientRect(hWnd, &rc);

    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd,
        &m_swapChain, &m_device, nullptr, &m_context
    );

    return SUCCEEDED(hr);
}


bool Renderer::CreateMainRenderTarget(int width, int height)
{
    ID3D11Texture2D* backBuffer;

    m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    m_device->CreateRenderTargetView(backBuffer, NULL, &m_rtv);

    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthDesc = {};

    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthTex = nullptr;

    m_device->CreateTexture2D(&depthDesc, nullptr, &depthTex);
    m_device->CreateDepthStencilView(depthTex, nullptr, &m_dsv);

    depthTex->Release();

    m_context->OMSetRenderTargets(1, &m_rtv, m_dsv);

    D3D11_VIEWPORT vp = {};

    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &vp);

    return true;
}

bool Renderer::CreateShaders()
{
    ID3DBlob* vsBlob = nullptr, * psBlob = nullptr, * err = nullptr;

    auto vsBytecode = ReadShaderFile("assets/shaders/lighting_vs.hlsl");

    HRESULT hr = D3DCompile(vsBytecode.data(), vsBytecode.size(), 0, 0, 0, "main", "vs_5_0", 0, 0, &vsBlob, &err);

    if (FAILED(hr)) 
    {
        if (err)
        {
            OutputDebugStringA((char*)err->GetBufferPointer());
        }

        return false;
    }

    m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    auto psBytecode = ReadShaderFile("assets/shaders/lighting_ps.hlsl");

    hr = D3DCompile(psBytecode.data(), psBytecode.size(), 0, 0, 0, "main", "ps_5_0", 0, 0, &psBlob, &err);

    if (FAILED(hr)) 
    {
        if (err)
        {
            OutputDebugStringA((char*)err->GetBufferPointer());
        }

        return false;
    }

    m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_lightingPixelShader);

    if (psBlob)
    {
        psBlob->Release();
    }

    auto unlitPsBytecode = ReadShaderFile("assets/shaders/unlit_ps.hlsl");

    hr = D3DCompile(unlitPsBytecode.data(), unlitPsBytecode.size(), 0, 0, 0, "main", "ps_5_0", 0, 0, &psBlob, &err);

    if (FAILED(hr))
    {
        if (err)
        {
            OutputDebugStringA((char*)err->GetBufferPointer());
        }

        return false;
    }

    m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_unlitPixelShader);

    if (psBlob)
    {
        psBlob->Release();
    }

    auto outlinePsBytecode = ReadShaderFile("assets/shaders/outline_ps.hlsl");

    hr = D3DCompile(outlinePsBytecode.data(), outlinePsBytecode.size(), 0, 0, 0, "main", "ps_5_0", 0, 0, &psBlob, &err);

    if (FAILED(hr)) 
    {
        if (err)
        {
            OutputDebugStringA((char*)err->GetBufferPointer());
        }

        return false;
    }

    m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_outlinePixelShader);

    if (psBlob)
    {
        psBlob->Release();
    }

    if (vsBlob)
    {
        vsBlob->Release();
    }

    return true;
}

bool Renderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC cbd = { sizeof(ConstantBufferData), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER };

    return SUCCEEDED(m_device->CreateBuffer(&cbd, nullptr, &m_constantBuffer));
}

bool Renderer::CreateRasterizerStates()
{
    D3D11_RASTERIZER_DESC rd = {};

    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.DepthClipEnable = TRUE;

    m_device->CreateRasterizerState(&rd, &m_rasterState);

    D3D11_RASTERIZER_DESC ord = {};

    ord.FillMode = D3D11_FILL_SOLID;
    ord.CullMode = D3D11_CULL_FRONT;
    ord.DepthClipEnable = TRUE;

    m_device->CreateRasterizerState(&ord, &m_outlineRasterState);

    return m_rasterState && m_outlineRasterState;
}

bool Renderer::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC ld[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    auto vsBytecode = ReadShaderFile("assets/shaders/lighting_vs.hlsl");

    ID3DBlob* vsBlob = nullptr;

    D3DCompile(vsBytecode.data(), vsBytecode.size(), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);

    if (!vsBlob)
    {
        return false;
    }

    HRESULT hr = m_device->CreateInputLayout(ld, _countof(ld), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    vsBlob->Release();

    return SUCCEEDED(hr);
}


bool Renderer::CreateMeshes()
{
    m_meshes[ShapeType::Cube].Create(m_device, Mesh::Geometry::CreateCube());

    m_meshes[ShapeType::Pyramid].Create(m_device, Mesh::Geometry::CreatePyramid());

    m_meshes[ShapeType::Sphere].Create(m_device, Mesh::Geometry::CreateSphere(0.5f, 20, 20));

    return true;
}

void Renderer::CreateGrid()
{
    std::vector<Vertex> gridLines;

    int gridSize = 20;

    DirectX::XMFLOAT3 gridColor = { 0.4f, 0.4f, 0.4f };

    for (int i = -gridSize; i <= gridSize; ++i)
    {
        gridLines.push_back({ { (float)-gridSize, 0.0f, (float)i }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)gridSize, 0.0f, (float)i }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)i, 0.0f, (float)-gridSize }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)i, 0.0f, (float)gridSize }, gridColor, { 0, 1, 0 } });
    };

    m_gridVertexCount = (UINT)gridLines.size();

    D3D11_BUFFER_DESC gvbd = { (UINT)(sizeof(Vertex) * m_gridVertexCount), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA ginit = { gridLines.data() };

    m_device->CreateBuffer(&gvbd, &ginit, &m_gridVertexBuffer);
}

bool Renderer::CreateDepthStencilStates()
{
    D3D11_DEPTH_STENCIL_DESC stencilWriteDesc = {};

    stencilWriteDesc.DepthEnable = TRUE;
    stencilWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    stencilWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;
    stencilWriteDesc.StencilEnable = TRUE;
    stencilWriteDesc.StencilWriteMask = 0xFF; 

    stencilWriteDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    stencilWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    stencilWriteDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    stencilWriteDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; 
    stencilWriteDesc.BackFace = stencilWriteDesc.FrontFace;

    HRESULT hr = m_device->CreateDepthStencilState(&stencilWriteDesc, &m_stencilWriteState);

    if (FAILED(hr))
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC stencilTestDesc = {};

    stencilTestDesc.DepthEnable = TRUE;
    stencilTestDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
    stencilTestDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    stencilTestDesc.StencilEnable = TRUE;
    stencilTestDesc.StencilReadMask = 0xFF; 
    stencilTestDesc.StencilWriteMask = 0x00; 
    stencilTestDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    stencilTestDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    stencilTestDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    stencilTestDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL; 
    stencilTestDesc.BackFace = stencilTestDesc.FrontFace;

    hr = m_device->CreateDepthStencilState(&stencilTestDesc, &m_stencilTestState);

    return SUCCEEDED(hr);
}