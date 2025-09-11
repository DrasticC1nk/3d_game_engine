#include "Mesh.h"

using namespace DirectX;

Mesh::Mesh() {}

Mesh::~Mesh()
{
    Release();
}

void Mesh::Release()
{
    if (vertexBuffer)
    {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    if (indexBuffer)
    {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }
}

bool Mesh::Create(ID3D11Device* device, const MeshData& data)
{
    Release();

    indexCount = (UINT)data.indices.size();

    D3D11_BUFFER_DESC vbd = {};

    vbd.ByteWidth = sizeof(Vertex) * (UINT)data.vertices.size(); 
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinit = {};

    vinit.pSysMem = data.vertices.data(); 

    if (FAILED(device->CreateBuffer(&vbd, &vinit, &vertexBuffer)))
    {
        return false;
    }

    D3D11_BUFFER_DESC ibd = {};

    ibd.ByteWidth = sizeof(UINT) * (UINT)data.indices.size();
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinit = {};

    iinit.pSysMem = data.indices.data(); 

    if (FAILED(device->CreateBuffer(&ibd, &iinit, &indexBuffer)))
    {
        return false;
    }

    return true;
}

MeshData Mesh::Geometry::CreateCube()
{
    MeshData data;

    data.vertices =
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

    data.indices = {
        0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23
    };

    return data;
}

MeshData Mesh::Geometry::CreatePyramid()
{
    MeshData data;

    data.vertices = 
    {
        { {-0.5f, 0.0f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { { 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { {-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
        { {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f, 0.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f} },
        { {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} }
    };

    data.indices = { 0, 2, 1, 0, 3, 2, 3, 4, 2, 2, 5, 1, 1, 6, 0, 0, 7, 3 };

    return data;
}

MeshData Mesh::Geometry::CreateSphere(float radius, UINT sliceCount, UINT stackCount)
{
    MeshData data;

    data.vertices.push_back({ {0.0f, radius, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });

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

            v.color = { v.normal.x, v.normal.y, v.normal.z };

            data.vertices.push_back(v);
        }
    }

    data.vertices.push_back({ {0.0f, -radius, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} });

    for (UINT i = 1; i <= sliceCount; ++i) 
    {
        data.indices.push_back(0);
        data.indices.push_back(i + 1);
        data.indices.push_back(i);
    }

    UINT baseIndex = 1;
    UINT ringVertexCount = sliceCount + 1;

    for (UINT i = 0; i < stackCount - 2; ++i) 
    {
        for (UINT j = 0; j < sliceCount; ++j) 
        {
            data.indices.push_back(baseIndex + i * ringVertexCount + j);
            data.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            data.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            data.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            data.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            data.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    UINT southPoleIndex = (UINT)data.vertices.size() - 1;

    baseIndex = southPoleIndex - ringVertexCount;

    for (UINT i = 0; i < sliceCount; ++i) 
    {
        data.indices.push_back(southPoleIndex);
        data.indices.push_back(baseIndex + i);
        data.indices.push_back(baseIndex + i + 1);
    }

    return data;
}