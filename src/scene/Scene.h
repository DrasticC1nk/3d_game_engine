#pragma once

#include <vector>
#include <string>
#include "GameObject.h"
#include "Camera.h"

class Scene
{
public:

    std::vector<GameObject> m_gameObjects;

    int m_selectedObjectIndex = -1;
    int m_nextObjectID = 1;

    Scene();

    void AddObject(ShapeType type);
    void RemoveObject(int index);

    void SelectObject(HWND hwnd, int mouseX, int mouseY);

    Camera& GetCamera() 
    { 
        return m_camera; 
    }

    std::vector<GameObject>& GetGameObjects() 
    { 
        return m_gameObjects; 
    }

    int GetSelectedObjectIndex() const 
    { 
        return m_selectedObjectIndex; 
    }
    void SetSelectedObjectIndex(int index) 
    { 
        m_selectedObjectIndex = index; 
    }

private:

    Camera m_camera;
};
