#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
    DirectX::XMFLOAT3 normal;
};

enum class ShapeType
{
    Cube,
    Pyramid,
    Sphere
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
};


class Mesh
{
public:

    Mesh();
    ~Mesh();

    bool Create(ID3D11Device* device, const MeshData& data);

    void Release();

    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    UINT indexCount = 0;

    class Geometry
    {
    public:

        static MeshData CreateCube();
        static MeshData CreatePyramid();
        static MeshData CreateSphere(float radius, UINT sliceCount, UINT stackCount);
    };
};