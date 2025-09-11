#include "Scene.h"
#include <fstream>
#include <sstream>
#include <windows.h> 

bool IntersectRayAABB(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDir, DirectX::XMVECTOR aabbMin, DirectX::XMVECTOR aabbMax, float& dist)
{
    using namespace DirectX;

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


Scene::Scene() {}

void Scene::AddObject(ShapeType type)
{
    m_gameObjects.emplace_back(m_nextObjectID++, type);
}

void Scene::RemoveObject(int index)
{
    if (index >= 0 && index < m_gameObjects.size())
    {
        m_gameObjects.erase(m_gameObjects.begin() + index);
        m_selectedObjectIndex = -1;
    }
}

void Scene::SelectObject(HWND hwnd, int mouseX, int mouseY)
{
    if (m_gameObjects.empty())
    {
        return;
    }

    RECT clientRect;

    GetClientRect(hwnd, &clientRect);

    using namespace DirectX;

    XMMATRIX proj = m_camera.GetProjectionMatrix();
    XMMATRIX view = m_camera.GetViewMatrix();
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);

    float ndcX = (2.0f * mouseX / (clientRect.right - clientRect.left)) - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY / (clientRect.bottom - clientRect.top));

    XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 1.0f, 1.0f), invViewProj) - m_camera.GetPosition());

    m_selectedObjectIndex = -1;

    float closestDist = FLT_MAX;

    for (int i = 0; i < m_gameObjects.size(); ++i)
    {
        GameObject& obj = m_gameObjects[i];
        XMVECTOR objPosVec = XMLoadFloat3(&obj.position);
        XMVECTOR objScaleVec = XMLoadFloat3(&obj.scale);
        XMVECTOR halfSize = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) * objScaleVec;

        float dist;

        if (IntersectRayAABB(m_camera.GetPosition(), rayDir, objPosVec - halfSize, objPosVec + halfSize, dist))
        {
            if (dist < closestDist)
            {
                closestDist = dist;

                m_selectedObjectIndex = i;
            }
        }
    }
}
