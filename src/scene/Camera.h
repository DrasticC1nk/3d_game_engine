#pragma once

#include <DirectXMath.h>

class Camera
{
public:

    Camera();

    void Update(const bool keys[]);
    void UpdateMouse(float dx, float dy);

    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjectionMatrix() const;
    DirectX::XMVECTOR GetPosition() const { return m_position; }

private:

    DirectX::XMVECTOR m_position = DirectX::XMVectorSet(8.0f, 6.0f, -8.0f, 1.0f);
    DirectX::XMVECTOR m_forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    float m_aspectRatio = 1280.0f / 720.0f; 
};