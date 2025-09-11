#pragma once

#include <string>
#include <DirectXMath.h>
#include "../Renderer/Mesh.h" 

struct GameObject
{
    int id;

    std::string name;

    ShapeType type;

    DirectX::XMFLOAT3 position = { 0.0f, 0.5f, 0.0f };
    DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

    bool enableAutoRotation = false;
    bool applyLighting = false;

    float autoRotationAngle = 0.0f;

    GameObject(int unique_id, ShapeType shape_type) : id(unique_id), type(shape_type)
    {
        switch (type)
        {
            case ShapeType::Cube:   

                name = "Cube " + std::to_string(id); break;

            case ShapeType::Pyramid: 

                name = "Pyramid " + std::to_string(id); break;

            case ShapeType::Sphere:  

                name = "Sphere " + std::to_string(id); break;
        }
    }

    DirectX::XMMATRIX GetTransformMatrix() const
    {
        using namespace DirectX;

        XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
        XMMATRIX autoRotMat = XMMatrixRotationY(autoRotationAngle) * XMMatrixRotationX(autoRotationAngle * 0.5f);
        XMMATRIX transMat = XMMatrixTranslation(position.x, position.y, position.z);

        return scaleMat * rotMat * autoRotMat * transMat;
    }
};
