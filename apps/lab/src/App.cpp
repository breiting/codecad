#include "App.hpp"

#include <imgui.h>

#include "GLFW/glfw3.h"
#include "pure/PureBounds.hpp"

using namespace pure;

App::App() = default;

bool App::Initialize(int width, int height, const std::string& title) {
    m_Controller = std::make_unique<PureController>();
    if (!m_Controller->Initialize(width, height, title.c_str())) return false;

    m_Scene = std::make_shared<PureScene>();

    m_Controller->SetMouseMoveHandler([this](double x, double y) { m_Measure.OnMouseMove(x, y); });
    m_Controller->SetMouseButtonHandler([this](int button, int action, int mods) {
        bool pressed = (action == GLFW_PRESS);
        bool shift = (mods & GLFW_MOD_SHIFT) != 0;
        m_Measure.OnMouseButton(button, pressed, shift);
    });

    m_Controller->SetKeyPressedHandler([this](int key, int /*mods*/) {
        switch (key) {
            case GLFW_KEY_1:
                if (!m_Scenarios.empty()) SwitchTo(0);
                break;
            case GLFW_KEY_2:
                if (m_Scenarios.size() > 1) SwitchTo(1);
                break;
            case GLFW_KEY_3:
                if (m_Scenarios.size() > 2) SwitchTo(2);
                break;
            case GLFW_KEY_W:
                m_Controller->ToggleWireframe();
                break;
            case GLFW_KEY_S:
                SaveSTL();
                break;
            case GLFW_KEY_R:
                Rebuild();
                break;

            default:
                break;
        }

        m_Measure.OnKey(key, true);
    });

    m_Picker = std::make_unique<PurePicker>();
    m_Picker->SetScene(m_Scene.get());
    m_Picker->SetSnapPixels(8.0f);  // 8px Snapradius
    m_Measure.SetPicker(m_Picker.get());
    m_Measure.SetMode(pure::MeasureMode::PointToPoint);  // default

    return true;
}

void App::Run() {
    if (m_Scenarios.empty()) {
        // empty scene is fine, but let’s log
        std::puts("[ccad-lab] No scenarios registered.");
    } else {
        SwitchTo(0);
    }

    // Set camera only upon first load
    PureBounds bounds;
    if (m_Scene->ComputeBounds(bounds)) {
        m_Controller->Camera()->FitToBounds(bounds, 1.12f);
    }

    while (!m_Controller->ShouldClose()) {
        m_Controller->BeginFrame();
        m_Controller->DrawGui();
        m_Controller->RenderScene(m_Scene);

        glm::mat4 view = m_Controller->Camera()->View();
        glm::mat4 proj = m_Controller->Camera()->Projection();

        m_Picker->SetViewProj(view, proj);
        m_Picker->SetViewport(m_Controller->GetFramebufferWidth(), m_Controller->GetFramebufferHeight());

        m_Measure.DrawOverlay(
            ImGui::GetForegroundDrawList(), proj * view,
            {float(m_Controller->GetFramebufferWidth()), float(m_Controller->GetFramebufferHeight())});

        // if (auto& r = m_Measure.Result()) {
        //     printf("%5.2f\n", r->distance);
        //     // print r->distance, r->p0, r->p1 etc.
        // }
        m_Controller->EndFrame();
    }
}

void App::Shutdown() {
    if (m_Controller) m_Controller->Shutdown();
}

void App::AddScenario(const ScenarioPtr& s) {
    m_Scenarios.push_back(s);
}

void App::Rebuild() {
    if (m_Scenarios.empty()) return;
    m_Scene->Clear();
    m_Scenarios[m_Current]->Build(m_Scene);
}

void App::SaveSTL() {
    if (m_Scenarios.empty()) return;
    m_Scenarios[m_Current]->SaveSTL("model");
}

void App::SwitchTo(size_t index) {
    if (index >= m_Scenarios.size()) return;
    m_Current = index;
    Rebuild();
}
