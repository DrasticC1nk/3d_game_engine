#include "Camera.h"
#include <windows.h> 
#include <algorithm> 

using namespace DirectX;
using namespace std;

Camera::Camera()
{
    XMVECTOR target = XMVectorZero();
    XMVECTOR direction = XMVector3Normalize(target - m_position);
    XMFLOAT3 dir;

    XMStoreFloat3(&dir, direction);

    m_yaw = atan2f(dir.x, dir.z);
    m_pitch = asinf(dir.y);
}

void Camera::Update(const bool keys[])
{
    XMMATRIX camRot = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);

    m_forward = XMVector3Normalize(XMVector3TransformCoord(XMVectorSet(0, 0, 1, 0), camRot));

    XMVECTOR camRight = XMVector3Normalize(XMVector3Cross(m_up, m_forward));

    float speed = 0.025f;

    if (keys['W'])
    {
        m_position += m_forward * speed;
    }
    if (keys['S'])
    {
        m_position -= m_forward * speed;
    }
    if (keys['A'])
    {
        m_position -= camRight * speed;
    }
    if (keys['D'])
    {
        m_position += camRight * speed;
    }
    if (keys[VK_SPACE])
    {
        m_position += m_up * speed;
    }
    if (keys[VK_CONTROL])
    {
        m_position -= m_up * speed;
    }
}

void Camera::UpdateMouse(float dx, float dy)
{
    float sens = 0.002f;

    m_yaw += dx * sens;
    m_pitch += dy * sens;
    m_pitch = max(-XM_PIDIV2 + 0.1f, min(XM_PIDIV2 - 0.1f, m_pitch));
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
    return XMMatrixLookAtLH(m_position, m_position + m_forward, m_up);
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() const
{
    return XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.1f, 100.0f);
}