#define NOMINMAX

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>      
#include <Commdlg.h>
#include <limits>
#include <sstream>
#include <map>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

using namespace DirectX;
using namespace std;

HWND hWnd = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* rtv = nullptr;
ID3D11DepthStencilView* dsv = nullptr;
ID3D11InputLayout* inputLayout = nullptr;
ID3D11Buffer* vertexBuffer = nullptr;
ID3D11Buffer* indexBuffer = nullptr;
ID3D11Buffer* constantBuffer = nullptr;
ID3D11VertexShader* vertexShader = nullptr;
ID3D11PixelShader* pixelShader = nullptr;
ID3D11RasterizerState* rasterState = nullptr;
ID3D11Buffer* gridVertexBuffer = nullptr;
UINT gridVertexCount = 0;
ID3D11PixelShader* outlinePixelShader = nullptr;
ID3D11RasterizerState* outlineRasterState = nullptr;
ID3D11PixelShader* greyPixelShader = nullptr;

bool keys[256] = { false };

XMVECTOR g_cameraPos = XMVectorSet(8.0f, 6.0f, -8.0f, 1.0f);
XMVECTOR g_cameraForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR g_cameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

float g_yaw = 0.0f;
float g_pitch = 0.0f;
bool g_mouseCaptured = false;

enum class ShapeType
{
    Cube,
    Pyramid,
    Sphere
};

struct Mesh
{
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;

    UINT indexCount = 0;

    ~Mesh()
    {
        if (vertexBuffer)
        {
            vertexBuffer->Release();
        }
        if (indexBuffer)
        {
            indexBuffer->Release();
        }
    }
};

map<ShapeType, Mesh> g_meshes;

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT3 color;
    XMFLOAT3 normal;
};

struct ConstantBufferData
{
    XMMATRIX model, view, projection;
    XMFLOAT3 lightPos;
    float shininess;
    XMFLOAT3 viewPos;
    float pad1;
    XMFLOAT4 lightColor;
};

struct SceneObject
{
    int id;

    string name;

    ShapeType type;

    XMFLOAT3 position = { 0.0f, 0.5f, 0.0f };
    XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

    bool enableAutoRotation = false;
    bool applyLighting = false;

    float autoRotationAngle = 0.0f;

    SceneObject(int unique_id, ShapeType shape_type) : id(unique_id), type(shape_type)
    {
        switch (type)
        {
        case ShapeType::Cube:

            name = "Cube " + to_string(id); break;

        case ShapeType::Pyramid:

            name = "Pyramid " + to_string(id); break;

        case ShapeType::Sphere:

            name = "Sphere " + to_string(id); break;
        }
    }
};

vector<SceneObject> g_sceneObjects;

int g_selectedObjectIndex = -1;
int g_nextObjectID = 1;

void CreateCube(vector<Vertex>& outVertices, vector<UINT>& outIndices)
{
    outVertices =
    {
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
        { {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
        { { 0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        { { 0.5f, 0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        { {-0.5f, 0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
        { {-0.5f, 0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
        { { 0.5f, 0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
        { { 0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
        { {-0.5f, 0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
        { {-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f} },
        { { 0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f} },
        { { 0.5f, 0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f} }
    };

    outIndices =
    {
        0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23
    };
}

void CreatePyramid(vector<Vertex>& outVertices, vector<UINT>& outIndices)
{
    outVertices =
    {
        { {-0.5f, 0.0f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { {-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },

        { {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        { {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }
    };

    outIndices =
    {
        0, 1, 2, 0, 2, 3, 3, 2, 4, 2, 1, 5, 1, 0, 6, 0, 3, 7
    };
}

void CreateSphere(vector<Vertex>& outVertices, vector<UINT>& outIndices, float radius, UINT sliceCount, UINT stackCount)
{
    outVertices.clear();
    outIndices.clear();

    outVertices.push_back({ {0.0f, radius, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });

    float phiStep = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    for (UINT i = 1; i <= stackCount - 1; ++i)
    {
        float phi = i * phiStep;

        for (UINT j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex v;

            v.pos.x = radius * sinf(phi) * cosf(theta);
            v.pos.y = radius * cosf(phi);
            v.pos.z = radius * sinf(phi) * sinf(theta);

            XMStoreFloat3(&v.normal, XMVector3Normalize(XMLoadFloat3(&v.pos)));

            v.color = { v.normal.x, v.normal.y, v.normal.z }; // Color based on normal

            outVertices.push_back(v);
        }
    }

    outVertices.push_back({ {0.0f, -radius, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} });

    for (UINT i = 1; i <= sliceCount; ++i)
    {
        outIndices.push_back(0);
        outIndices.push_back(i + 1);
        outIndices.push_back(i);
    }

    UINT baseIndex = 1;
    UINT ringVertexCount = sliceCount + 1;

    for (UINT i = 0; i < stackCount - 2; ++i)
    {
        for (UINT j = 0; j < sliceCount; ++j)
        {
            outIndices.push_back(baseIndex + i * ringVertexCount + j);
            outIndices.push_back(baseIndex + i * ringVertexCount + j + 1);
            outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            outIndices.push_back(baseIndex + i * ringVertexCount + j + 1);
            outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    UINT southPoleIndex = (UINT)outVertices.size() - 1;

    baseIndex = southPoleIndex - ringVertexCount;

    for (UINT i = 0; i < sliceCount; ++i)
    {
        outIndices.push_back(southPoleIndex);
        outIndices.push_back(baseIndex + i);
        outIndices.push_back(baseIndex + i + 1);
    }
}

const char* vsSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        matrix model;
        matrix view;
        matrix projection;

        float3 lightPos;
        float shininess;
        float3 viewPos;
        float4 lightColor;
    };

    struct VS_IN
    {
        float3 pos : POSITION;
        float3 col : COLOR;
        float3 norm : NORMAL;
    };

    struct VS_OUT
    {
        float4 pos : SV_POSITION;
        float3 col : COLOR;
        float3 fragPos : TEXCOORD0;
        float3 worldNormal : NORMAL;
    };

    VS_OUT main(VS_IN input)
    {
        VS_OUT output;

        float4 worldPos = mul(float4(input.pos, 1.0f), model);

        output.fragPos = worldPos.xyz;
        output.worldNormal = mul(input.norm, (float3x3)model);

        output.pos = mul(worldPos, view);
        output.pos = mul(output.pos, projection);
        
        output.col = input.col;
        
        return output;
    }
)";

const char* psSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        matrix model;
        matrix view;
        matrix projection;
        float3 lightPos;
        float shininess;
        float3 viewPos;
        float4 lightColor;
    };

    struct PS_IN
    {
        float4 pos : SV_POSITION;
        float3 col : COLOR;
        float3 fragPos : TEXCOORD0;
        float3 worldNormal : NORMAL;
    };

    float4 main(PS_IN input) : SV_TARGET
    {
        float ambientStrength = 0.2f;
        float3 ambient = ambientStrength * lightColor.rgb * input.col;

        float3 norm = normalize(input.worldNormal);
        float3 lightDir = normalize(lightPos - input.fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        float3 diffuse = diff * lightColor.rgb * input.col;

        float3 viewDir = normalize(viewPos - input.fragPos);
        float3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        float3 specular = spec * lightColor.rgb;
        
        float3 result = ambient + diffuse + specular;

        return float4(result, 1.0f);
    }
)";

const char* psOutlineSource = R"(
    float4 main() : SV_TARGET
    {
        // Solid orange color for the outline
        return float4(1.0f, 0.65f, 0.0f, 1.0f);
    }
)";

const char* psGreySource = R"(
    float4 main() : SV_TARGET
    {
        return float4(0.5f,0.5f,0.5f,1.0f);
    }
)";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void UpdateCamera();
void Render();
void InitD3D();
void InitWindow(HINSTANCE hInstance);
void InitCamera();
void PrintShaderError(ID3DBlob* e);
void ResizeSwapChain(int width, int height);
void SaveScene();
void OpenScene();
void WriteSceneToFile(const string& filepath);
void ReadSceneFromFile(const string& filepath);

bool IntersectRayAABB(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR aabbMin, XMVECTOR aabbMax, float& dist);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    InitWindow(hInstance);
    InitCamera();
    InitD3D();

    MSG msg = {};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }

    ImGui_ImplDX11_Shutdown();

    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    if (greyPixelShader)
    {
        greyPixelShader->Release();
    }
    if (outlinePixelShader)
    {
        outlinePixelShader->Release();
    }
    if (outlineRasterState)
    {
        outlineRasterState->Release();
    }
    if (gridVertexBuffer)
    {
        gridVertexBuffer->Release();
    }
    if (rasterState)
    {
        rasterState->Release();
    }
    if (vertexShader)
    {
        vertexShader->Release();
    }
    if (pixelShader)
    {
        pixelShader->Release();
    }
    if (inputLayout)
    {
        inputLayout->Release();
    }
    if (vertexBuffer)
    {
        vertexBuffer->Release();
    }
    if (indexBuffer)
    {
        indexBuffer->Release();
    }
    if (constantBuffer)
    {
        constantBuffer->Release();
    }
    if (rtv)
    {
        rtv->Release();
    }
    if (dsv)
    {
        dsv->Release();
    }
    if (swapChain)
    {
        swapChain->Release();
    }
    if (context)
    {
        context->Release();
    }
    if (device)
    {
        device->Release();
    }

    g_meshes.clear();

    return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_SIZE:
    {
        if (device != nullptr && wParam != SIZE_MINIMIZED)
        {
            ResizeSwapChain(LOWORD(lParam), HIWORD(lParam));
        }

        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        if (ImGui::GetIO().WantCaptureMouse || g_sceneObjects.empty())
        {
            return 0;
        }

        RECT clientRect;

        GetClientRect(hWnd, &clientRect);

        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)(clientRect.right - clientRect.left) / (clientRect.bottom - clientRect.top), 0.1f, 100.0f);
        XMMATRIX view = XMMatrixLookAtLH(g_cameraPos, g_cameraPos + g_cameraForward, g_cameraUp);
        XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);

        float ndcX = (2.0f * mouseX / (clientRect.right - clientRect.left)) - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY / (clientRect.bottom - clientRect.top));

        XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 1.0f, 1.0f), invViewProj) - g_cameraPos);

        g_selectedObjectIndex = -1;

        float closestDist = FLT_MAX;

        for (int i = 0; i < g_sceneObjects.size(); ++i)
        {
            SceneObject& obj = g_sceneObjects[i];

            XMVECTOR objPosVec = XMLoadFloat3(&obj.position);
            XMVECTOR objScaleVec = XMLoadFloat3(&obj.scale);
            XMVECTOR halfSize = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) * objScaleVec;

            float dist;

            if (IntersectRayAABB(g_cameraPos, rayDir, objPosVec - halfSize, objPosVec + halfSize, dist))
            {
                if (dist < closestDist)
                {
                    closestDist = dist;

                    g_selectedObjectIndex = i;
                }
            }
        }
        return 0;
    }
    case WM_RBUTTONDOWN:
    {
        if (ImGui::GetIO().WantCaptureMouse)
        {
            return 0;
        }

        g_mouseCaptured = true;

        SetCapture(hWnd);
        ShowCursor(FALSE);

        RECT rect;

        GetClientRect(hWnd, &rect);

        POINT center = { rect.right / 2, rect.bottom / 2 };

        ClientToScreen(hWnd, &center);
        SetCursorPos(center.x, center.y);

        return 0;
    }
    case WM_RBUTTONUP:
    {
        g_mouseCaptured = false;

        ReleaseCapture();
        ShowCursor(TRUE);

        return 0;
    }
    case WM_KEYDOWN:
    {
        if (wParam < 256)
        {
            keys[wParam] = true;
        }

        return 0;
    }
    case WM_KEYUP:
    {
        if (wParam < 256)
        {
            keys[wParam] = false;
        }

        return 0;
    }
    case WM_MOUSEMOVE:
    {
        if (g_mouseCaptured)
        {
            RECT rect;

            GetClientRect(hWnd, &rect);

            POINT center = { rect.right / 2, rect.bottom / 2 };

            ClientToScreen(hWnd, &center);

            POINT currentMousePos;

            GetCursorPos(&currentMousePos);

            float dx = (float)(currentMousePos.x - center.x);
            float dy = (float)(currentMousePos.y - center.y);

            if (dx != 0 || dy != 0)
            {
                float sens = 0.002f;

                g_yaw += dx * sens;
                g_pitch += dy * sens;
                g_pitch = max(-XM_PIDIV2 + 0.1f, min(XM_PIDIV2 - 0.1f, g_pitch));

                SetCursorPos(center.x, center.y);
            }
        }

        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);

        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void UpdateCamera()
{
    if (ImGui::GetIO().WantCaptureKeyboard)
    {
        return;
    }

    XMMATRIX camRot = XMMatrixRotationRollPitchYaw(g_pitch, g_yaw, 0);

    g_cameraForward = XMVector3Normalize(XMVector3TransformCoord(XMVectorSet(0, 0, 1, 0), camRot));

    XMVECTOR camRight = XMVector3Cross(g_cameraUp, g_cameraForward);

    float speed = 0.025f;

    if (keys['W'])
    {
        g_cameraPos += g_cameraForward * speed;
    }
    if (keys['S'])
    {
        g_cameraPos -= g_cameraForward * speed;
    }
    if (keys['A'])
    {
        g_cameraPos -= camRight * speed;
    }
    if (keys['D'])
    {
        g_cameraPos += camRight * speed;
    }
    if (keys[VK_SPACE])
    {
        g_cameraPos += g_cameraUp * speed;
    }
    if (keys[VK_CONTROL])
    {
        g_cameraPos -= g_cameraUp * speed;
    }
}

void Render()
{
    UpdateCamera();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                SaveScene();
            }
            if (ImGui::MenuItem("Open Scene"))
            {
                OpenScene();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::SetNextWindowPos(ImVec2(20, 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

    ImGui::Begin("Scene Controls");

    const char* items[] = { "Cube", "Pyramid", "Sphere" };

    static int currentItem = 0;

    ImGui::Combo("Shape to Add", &currentItem, items, IM_ARRAYSIZE(items));

    if (ImGui::Button("Add Shape"))
    {
        g_sceneObjects.emplace_back(g_nextObjectID++, static_cast<ShapeType>(currentItem));
    }

    ImGui::Separator();
    ImGui::Text("Scene Objects");

    for (int i = 0; i < g_sceneObjects.size(); ++i)
    {
        bool isSelected = (i == g_selectedObjectIndex);

        if (ImGui::Selectable(g_sceneObjects[i].name.c_str(), isSelected))
        {
            g_selectedObjectIndex = i;
        }
    }
    ImGui::End();

    if (g_selectedObjectIndex != -1)
    {
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        ImVec2 right_panel_pos = ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x - 320, main_viewport->WorkPos.y + 40);

        ImGui::SetNextWindowPos(right_panel_pos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);

        SceneObject& selectedObj = g_sceneObjects[g_selectedObjectIndex];

        ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("%s", selectedObj.name.c_str());
        ImGui::Separator();
        ImGui::SliderFloat3("Position", &selectedObj.position.x, -10.0f, 10.0f);
        ImGui::SliderFloat3("Rotation", &selectedObj.rotation.x, 0.0f, 360.0f);
        ImGui::SliderFloat3("Scale", &selectedObj.scale.x, 0.1f, 5.0f);
        ImGui::Separator();
        ImGui::Checkbox("Enable Auto-Rotation", &selectedObj.enableAutoRotation);
        ImGui::Checkbox("Apply Lighting Shader", &selectedObj.applyLighting);
        ImGui::Separator();

        if (ImGui::Button("Remove This Cube"))
        {
            g_sceneObjects.erase(g_sceneObjects.begin() + g_selectedObjectIndex);

            g_selectedObjectIndex = -1;
        }

        ImGui::End();
    }

    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    context->ClearRenderTargetView(rtv, clearColor);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, 0, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetConstantBuffers(0, 1, &constantBuffer);

    RECT rc;

    GetClientRect(hWnd, &rc);

    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)(rc.right - rc.left) / (rc.bottom - rc.top), 0.1f, 100.0f);
    XMMATRIX view = XMMatrixLookAtLH(g_cameraPos, g_cameraPos + g_cameraForward, g_cameraUp);

    ConstantBufferData cb = {};

    cb.view = XMMatrixTranspose(view);
    cb.projection = XMMatrixTranspose(proj);

    XMStoreFloat3(&cb.viewPos, g_cameraPos);

    cb.lightPos = XMFLOAT3(2, 10, -5);
    cb.lightColor = XMFLOAT4(1, 1, 1, 1);
    cb.shininess = 128.0f;

    UINT stride = sizeof(Vertex), offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (int i = 0; i < g_sceneObjects.size(); ++i)
    {
        SceneObject& obj = g_sceneObjects[i];

        Mesh& currentMesh = g_meshes[obj.type];

        context->IASetVertexBuffers(0, 1, &currentMesh.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(currentMesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (obj.enableAutoRotation)
        {
            obj.autoRotationAngle += 0.005f;
        }

        XMMATRIX scaleMat = XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
        XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(XMConvertToRadians(obj.rotation.x), XMConvertToRadians(obj.rotation.y), XMConvertToRadians(obj.rotation.z));
        XMMATRIX autoRotMat = XMMatrixRotationY(obj.autoRotationAngle) * XMMatrixRotationX(obj.autoRotationAngle * 0.5f);
        XMMATRIX transMat = XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        XMMATRIX modelMat = scaleMat * rotMat * autoRotMat * transMat;

        if (i == g_selectedObjectIndex)
        {
            context->RSSetState(outlineRasterState);
            context->PSSetShader(outlinePixelShader, 0, 0);

            XMMATRIX outlineScaleMat = XMMatrixScaling(1.08f, 1.08f, 1.08f);

            cb.model = XMMatrixTranspose(outlineScaleMat * modelMat);

            context->UpdateSubresource(constantBuffer, 0, 0, &cb, 0, 0);
            context->DrawIndexed(currentMesh.indexCount, 0, 0);
        }

        context->RSSetState(rasterState);
        context->PSSetShader(obj.applyLighting ? pixelShader : greyPixelShader, 0, 0);

        cb.model = XMMatrixTranspose(modelMat);

        context->UpdateSubresource(constantBuffer, 0, 0, &cb, 0, 0);
        context->DrawIndexed(currentMesh.indexCount, 0, 0);
    }

    context->IASetVertexBuffers(0, 1, &gridVertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->PSSetShader(greyPixelShader, 0, 0);

    cb.model = XMMatrixTranspose(XMMatrixIdentity());

    context->UpdateSubresource(constantBuffer, 0, 0, &cb, 0, 0);
    context->Draw(gridVertexCount, 0);

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swapChain->Present(1, 0);
}

void SaveScene()
{
    OPENFILENAMEA ofn;

    char szFile[260] = "scene.ini";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        WriteSceneToFile(ofn.lpstrFile);
    }
}

void OpenScene()
{
    OPENFILENAMEA ofn;

    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        ReadSceneFromFile(ofn.lpstrFile);
    }
}

void WriteSceneToFile(const string& filepath)
{
    ofstream outFile(filepath);

    if (!outFile.is_open())
    {
        MessageBox(hWnd, L"Failed to open file for writing.", L"Error", MB_OK);

        return;
    }

    outFile << "[Scene]" << endl;
    outFile << "NextObjectID=" << g_nextObjectID << endl;
    outFile << "ObjectCount=" << g_sceneObjects.size() << endl;
    outFile << endl;

    for (const auto& obj : g_sceneObjects)
    {
        outFile << "[Object_" << obj.id << "]" << endl;
        outFile << "Name=" << obj.name << endl;
        outFile << "ShapeType=" << static_cast<int>(obj.type) << endl;
        outFile << "Position=" << obj.position.x << " " << obj.position.y << " " << obj.position.z << endl;
        outFile << "Rotation=" << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << endl;
        outFile << "Scale=" << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << endl;
        outFile << "AutoRotate=" << obj.enableAutoRotation << endl;
        outFile << "UseLighting=" << obj.applyLighting << endl;
        outFile << endl;
    }

    size_t ini_size = 0;

    const char* imgui_ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);

    outFile << "[ImGuiLayout]" << endl;
    outFile << string(imgui_ini_data, ini_size) << endl;

    outFile.close();
}

void ReadSceneFromFile(const string& filepath)
{
    ifstream inFile(filepath);

    if (!inFile.is_open())
    {
        MessageBox(hWnd, L"Failed to open file for reading.", L"Error", MB_OK);
        return;
    }

    g_sceneObjects.clear();
    g_selectedObjectIndex = -1;

    string line;
    string layout_data; 

    SceneObject* currentObject = nullptr;

    bool isReadingLayout = false;

    while (getline(inFile, line))
    {

        if (line.find("[Scene]") != string::npos)
        {
            currentObject = nullptr;

            isReadingLayout = false;
        }
        else if (line.find("[Object_") != string::npos)
        {
            isReadingLayout = false;

            string idStr = line.substr(line.find("_") + 1, line.find("]") - line.find("_") - 1);

            int currentId = stoi(idStr);

            g_sceneObjects.emplace_back(currentId, ShapeType::Cube);

            currentObject = &g_sceneObjects.back();
        }
        else if (line.find("[ImGuiLayout]") != string::npos)
        {
            currentObject = nullptr;

            isReadingLayout = true;

            continue; 
        }

        if (isReadingLayout)
        {
            layout_data += line + "\n";
        }
        else if (currentObject != nullptr)
        {
            size_t separatorPos = line.find("=");

            if (separatorPos == string::npos)
            {
                continue;
            }

            string key = line.substr(0, separatorPos);
            string value = line.substr(separatorPos + 1);
            stringstream ss(value);

            if (key == "Name") 
            { 
                currentObject->name = value; 
            }
            else if (key == "ShapeType") 
            { 
                currentObject->type = static_cast<ShapeType>(stoi(value)); 
            }
            else if (key == "Position") 
            { 
                ss >> currentObject->position.x >> currentObject->position.y >> currentObject->position.z; 
            }
            else if (key == "Rotation") 
            { 
                ss >> currentObject->rotation.x >> currentObject->rotation.y >> currentObject->rotation.z; 
            }
            else if (key == "Scale") 
            { 
                ss >> currentObject->scale.x >> currentObject->scale.y >> currentObject->scale.z; 
            }
            else if (key == "AutoRotate") 
            { 
                currentObject->enableAutoRotation = stoi(value); 
            }
            else if (key == "UseLighting") 
            { 
                currentObject->applyLighting = stoi(value); 
            }
        }
        else
        {
            if (line.find("NextObjectID=") != string::npos)
            {
                g_nextObjectID = stoi(line.substr(line.find("=") + 1));
            }
        }
    }

    if (!layout_data.empty())
    {
        ImGui::LoadIniSettingsFromMemory(layout_data.c_str(), layout_data.size());
    }

    inFile.close();
}

void InitWindow(HINSTANCE hInstance)
{
    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DX11WindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    hWnd = CreateWindow(L"DX11WindowClass", L"DX11 Scene Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOW);
}

void ResizeSwapChain(int width, int height)
{
    if (!swapChain)
    {
        return;
    }

    if (rtv)
    {
        rtv->Release();
    }
    if (dsv)
    {
        dsv->Release();
    }

    context->OMSetRenderTargets(0, 0, 0);

    swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    ID3D11Texture2D* backBuffer;

    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, NULL, &rtv);

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

    device->CreateTexture2D(&depthDesc, nullptr, &depthTex);
    device->CreateDepthStencilView(depthTex, nullptr, &dsv);

    depthTex->Release();

    context->OMSetRenderTargets(1, &rtv, dsv);

    D3D11_VIEWPORT vp = {};

    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}

void InitD3D()
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

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);

    GetClientRect(hWnd, &rc);

    ResizeSwapChain(rc.right - rc.left, rc.bottom - rc.top);

    vector<Vertex> tempVertices;
    vector<UINT> tempIndices;

    CreateCube(tempVertices, tempIndices);

    g_meshes[ShapeType::Cube].indexCount = tempIndices.size();

    D3D11_BUFFER_DESC vbd = { (UINT)(sizeof(Vertex) * tempVertices.size()), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA vinit = { tempVertices.data() };

    device->CreateBuffer(&vbd, &vinit, &g_meshes[ShapeType::Cube].vertexBuffer);

    D3D11_BUFFER_DESC ibd = { (UINT)(sizeof(UINT) * tempIndices.size()), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER };
    D3D11_SUBRESOURCE_DATA iinit = { tempIndices.data() };

    device->CreateBuffer(&ibd, &iinit, &g_meshes[ShapeType::Cube].indexBuffer);

    CreatePyramid(tempVertices, tempIndices);

    g_meshes[ShapeType::Pyramid].indexCount = tempIndices.size();

    vbd.ByteWidth = (UINT)(sizeof(Vertex) * tempVertices.size());
    vinit.pSysMem = tempVertices.data();

    device->CreateBuffer(&vbd, &vinit, &g_meshes[ShapeType::Pyramid].vertexBuffer);

    ibd.ByteWidth = (UINT)(sizeof(UINT) * tempIndices.size());
    iinit.pSysMem = tempIndices.data();

    device->CreateBuffer(&ibd, &iinit, &g_meshes[ShapeType::Pyramid].indexBuffer);

    CreateSphere(tempVertices, tempIndices, 0.5f, 20, 20);

    g_meshes[ShapeType::Sphere].indexCount = tempIndices.size();

    vbd.ByteWidth = (UINT)(sizeof(Vertex) * tempVertices.size());
    vinit.pSysMem = tempVertices.data();

    device->CreateBuffer(&vbd, &vinit, &g_meshes[ShapeType::Sphere].vertexBuffer);

    ibd.ByteWidth = (UINT)(sizeof(UINT) * tempIndices.size());
    iinit.pSysMem = tempIndices.data();

    device->CreateBuffer(&ibd, &iinit, &g_meshes[ShapeType::Sphere].indexBuffer);

    D3D11_BUFFER_DESC cbd = { sizeof(ConstantBufferData), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER };

    device->CreateBuffer(&cbd, nullptr, &constantBuffer);

    vector<Vertex> gridLines;

    int gridSize = 20;

    XMFLOAT3 gridColor = { 0.4f, 0.4f, 0.4f };

    for (int i = -gridSize; i <= gridSize; ++i)
    {
        gridLines.push_back({ { (float)-gridSize, 0.0f, (float)i }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)gridSize, 0.0f, (float)i }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)i, 0.0f, (float)-gridSize }, gridColor, { 0, 1, 0 } });
        gridLines.push_back({ { (float)i, 0.0f, (float)gridSize }, gridColor, { 0, 1, 0 } });
    };

    gridVertexCount = gridLines.size();

    D3D11_BUFFER_DESC gvbd = { (UINT)(sizeof(Vertex) * gridVertexCount), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA ginit = { gridLines.data() };

    device->CreateBuffer(&gvbd, &ginit, &gridVertexBuffer);

    ID3DBlob* vsBlob = nullptr, * psBlob = nullptr, * err = nullptr;

    D3DCompile(vsSource, strlen(vsSource), 0, 0, 0, "main", "vs_5_0", 0, 0, &vsBlob, &err);

    if (err)
    {
        PrintShaderError(err);
    }

    D3DCompile(psSource, strlen(psSource), 0, 0, 0, "main", "ps_5_0", 0, 0, &psBlob, &err);

    if (err)
    {
        PrintShaderError(err);
    }

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    ID3DBlob* opsBlob = nullptr;

    D3DCompile(psOutlineSource, strlen(psOutlineSource), 0, 0, 0, "main", "ps_5_0", 0, 0, &opsBlob, &err);

    if (err)
    {
        PrintShaderError(err);
    }

    device->CreatePixelShader(opsBlob->GetBufferPointer(), opsBlob->GetBufferSize(), nullptr, &outlinePixelShader);

    if (opsBlob)
    {
        opsBlob->Release();
    }

    ID3DBlob* greyPsBlob = nullptr;

    D3DCompile(psGreySource, strlen(psGreySource), 0, 0, 0, "main", "ps_5_0", 0, 0, &greyPsBlob, &err);

    if (err)
    {
        PrintShaderError(err);
    }

    device->CreatePixelShader(greyPsBlob->GetBufferPointer(), greyPsBlob->GetBufferSize(), nullptr, &greyPixelShader);

    if (greyPsBlob)
    {
        greyPsBlob->Release();
    }

    D3D11_RASTERIZER_DESC ord = {};

    ord.FillMode = D3D11_FILL_SOLID;
    ord.CullMode = D3D11_CULL_FRONT;
    ord.DepthClipEnable = TRUE;

    device->CreateRasterizerState(&ord, &outlineRasterState);

    D3D11_INPUT_ELEMENT_DESC ld[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    device->CreateInputLayout(ld, _countof(ld), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

    if (vsBlob)
    {
        vsBlob->Release();
    }
    if (psBlob)
    {
        psBlob->Release();
    }

    D3D11_RASTERIZER_DESC rd = {};

    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.DepthClipEnable = TRUE;

    device->CreateRasterizerState(&rd, &rasterState);
    context->RSSetState(rasterState);

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();(void)io;
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);
}

void InitCamera()
{
    XMVECTOR t = XMVectorZero();
    XMVECTOR d = XMVector3Normalize(t - g_cameraPos);
    XMFLOAT3 dir;

    XMStoreFloat3(&dir, d);

    g_yaw = atan2f(dir.x, dir.z);
    g_pitch = asinf(dir.y);
}

void PrintShaderError(ID3DBlob* e)
{
    if (e)
    {
        cerr << "Shader compile error:\n" << (const char*)e->GetBufferPointer() << endl;
    }
}

bool IntersectRayAABB(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR aabbMin, XMVECTOR aabbMax, float& dist)
{
    XMVECTOR invDir = XMVectorReciprocal(rayDir);
    XMVECTOR t1 = (aabbMin - rayOrigin) * invDir;
    XMVECTOR t2 = (aabbMax - rayOrigin) * invDir;
    XMVECTOR tmin = XMVectorMin(t1, t2);
    XMVECTOR tmax = XMVectorMax(t1, t2);

    float t_near = max(XMVectorGetX(tmin), max(XMVectorGetY(tmin), XMVectorGetZ(tmin)));
    float t_far = min(XMVectorGetX(tmax), min(XMVectorGetY(tmax), XMVectorGetZ(tmax)));

    if (t_far < 0 || t_near > t_far)
    {
        return false;
    }

    dist = t_near > 0 ? t_near : t_far;

    return true;
}