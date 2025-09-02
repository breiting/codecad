#include "App.hpp"

using namespace pure;

App::App() = default;

bool App::Initialize(int width, int height, const std::string& title) {
    m_Controller = std::make_unique<PureController>();
    if (!m_Controller->Initialize(width, height, title.c_str())) return false;

    m_Scene = std::make_shared<PureScene>();

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
            case GLFW_KEY_R:
                Rebuild();
                break;

            default:
                break;
        }
    });

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
    glm::vec3 bmin, bmax;
    if (m_Scene->ComputeBounds(bmin, bmax)) {
        m_Controller->Camera().FitToBounds(bmin, bmax, 1.12f);
    }

    while (!m_Controller->ShouldClose()) {
        m_Controller->BeginFrame();
        m_Controller->DrawGui();
        m_Controller->RenderScene(m_Scene);
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

void App::SwitchTo(size_t index) {
    if (index >= m_Scenarios.size()) return;
    m_Current = index;
    Rebuild();
}
