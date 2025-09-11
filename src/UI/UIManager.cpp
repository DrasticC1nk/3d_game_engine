#include <fstream> 
#include <sstream> 
#include "UIManager.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_impl_win32.h"
#include "../../external/imgui/imgui_impl_dx11.h"
#include "../Scene/Scene.h"
#include <Commdlg.h>

UIManager::UIManager() {}
UIManager::~UIManager() {}

bool UIManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = NULL;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd))
    {
        return false;
    }
    if (!ImGui_ImplDX11_Init(device, context))
    {
        return false;
    }

    return true;
}

void UIManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

void UIManager::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
}

void UIManager::RenderUI(Scene& scene)
{
    ShowMenuBar(scene);
    ShowSceneControls(scene);
    ShowObjectProperties(scene);
}

void UIManager::EndFrame()
{
    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool UIManager::WantCaptureMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool UIManager::WantCaptureKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void UIManager::ShowMenuBar(Scene& scene)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                SaveScene(scene);
            }
            if (ImGui::MenuItem("Open Scene"))
            {
                OpenScene(scene);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UIManager::ShowSceneControls(Scene& scene)
{
    ImGui::SetNextWindowPos(ImVec2(20, 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene Controls");

    const char* items[] = { "Cube", "Pyramid", "Sphere" };
    static int currentItem = 0;

    ImGui::Combo("Shape to Add", &currentItem, items, IM_ARRAYSIZE(items));

    if (ImGui::Button("Add Shape"))
    {
        scene.AddObject(static_cast<ShapeType>(currentItem));
    }

    ImGui::Separator();
    ImGui::Text("Scene Objects");

    for (int i = 0; i < scene.GetGameObjects().size(); ++i)
    {
        bool isSelected = (i == scene.GetSelectedObjectIndex());

        if (ImGui::Selectable(scene.GetGameObjects()[i].name.c_str(), isSelected))
        {
            scene.SetSelectedObjectIndex(i);
        }
    }

    ImGui::End();
}

void UIManager::ShowObjectProperties(Scene& scene)
{
    int selectedIndex = scene.GetSelectedObjectIndex();

    if (selectedIndex == -1)
    {
        return;
    }

    GameObject& selectedObj = scene.GetGameObjects()[selectedIndex];

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    ImVec2 right_panel_pos = ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x - 320, main_viewport->WorkPos.y + 40);

    ImGui::SetNextWindowPos(right_panel_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);

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

    if (ImGui::Button("Remove This Object"))
    {
        scene.RemoveObject(selectedIndex);
    }

    ImGui::End();
}

void UIManager::SaveScene(Scene& scene)
{
    OPENFILENAMEA ofn;

    char szFile[260] = "scene.ini";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        WriteSceneAndUILayoutToFile(ofn.lpstrFile, scene);
    }
}

void UIManager::OpenScene(Scene& scene)
{
    OPENFILENAMEA ofn;

    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        LoadSceneAndUILayoutFromFile(ofn.lpstrFile, scene);
    }
}

void UIManager::WriteSceneAndUILayoutToFile(const std::string& filepath, Scene& scene)
{
    std::ofstream outFile(filepath);

    if (!outFile.is_open())
    {
        return;
    }

    outFile << "[Scene]" << std::endl;
    outFile << "NextObjectID=" << scene.m_nextObjectID << std::endl;
    outFile << "ObjectCount=" << scene.m_gameObjects.size() << std::endl;
    outFile << std::endl;

    for (const auto& obj : scene.m_gameObjects)
    {
        outFile << "[Object_" << obj.id << "]" << std::endl;
        outFile << "Name=" << obj.name << std::endl;
        outFile << "ShapeType=" << static_cast<int>(obj.type) << std::endl;
        outFile << "Position=" << obj.position.x << " " << obj.position.y << " " << obj.position.z << std::endl;
        outFile << "Rotation=" << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << std::endl;
        outFile << "Scale=" << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << std::endl;
        outFile << "AutoRotate=" << obj.enableAutoRotation << std::endl;
        outFile << "UseLighting=" << obj.applyLighting << std::endl;
        outFile << std::endl;
    }

    size_t ini_size = 0;

    const char* imgui_ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);

    outFile << "[ImGuiLayout]" << std::endl;
    outFile << std::string(imgui_ini_data, ini_size);

    outFile.close();
}

void UIManager::LoadSceneAndUILayoutFromFile(const std::string& filepath, Scene& scene)
{
    std::ifstream inFile(filepath);

    if (!inFile.is_open())
    {
        return;
    }

    scene.m_gameObjects.clear();
    scene.m_selectedObjectIndex = -1;

    std::string line;
    std::string layout_data;

    GameObject* currentObject = nullptr;

    bool isReadingLayout = false;

    while (getline(inFile, line))
    {
        if (line.find("[ImGuiLayout]") != std::string::npos)
        {
            isReadingLayout = true;

            continue;
        }

        if (isReadingLayout)
        {
            layout_data += line + "\n";
        }
        else
        {
            if (line.find("[Object_") != std::string::npos)
            {
                std::string idStr = line.substr(line.find("_") + 1, line.find("]") - line.find("_") - 1);

                int currentId = stoi(idStr);

                scene.m_gameObjects.emplace_back(currentId, ShapeType::Cube);

                currentObject = &scene.m_gameObjects.back();
            }
            else if (currentObject)
            {
                size_t separatorPos = line.find("=");

                if (separatorPos == std::string::npos)
                {
                    continue;
                }

                std::string key = line.substr(0, separatorPos);
                std::string value = line.substr(separatorPos + 1);
                std::stringstream ss(value);

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
            else if (line.find("NextObjectID=") != std::string::npos)
            {
                scene.m_nextObjectID = stoi(line.substr(line.find("=") + 1));
            }
        }
    }

    if (!layout_data.empty())
    {
        ImGui::LoadIniSettingsFromMemory(layout_data.c_str(), layout_data.size());
    }

    inFile.close();
}