#include "ProjectPanel.hpp"

#include <imgui.h>

ProjectPanel::ProjectPanel(io::Project& p) : m_Project(p) {
}
void ProjectPanel::SetOnSave(SaveCallback cb) {
    m_OnSave = std::move(cb);
}

void ProjectPanel::Draw() {
    bool changed = false;
    if (ImGui::CollapsingHeader("Meta", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= DrawMeta(m_Project.meta);
    }
    if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= DrawParams(m_Project.params);
    }

    if (changed) {
        m_Dirty = true;
        m_LastEdit = std::chrono::steady_clock::now();
    }

    // Debounced Save
    if (m_Dirty) {
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastEdit).count();
        if (ms > m_DebounceMs && m_OnSave) {
            m_OnSave(m_Project);
            m_Dirty = false;
        }
    }

    // TODO: Currently the project is always saved
    // if (ImGui::Button("Save")) {
    //     if (m_OnSave) m_OnSave(m_Project);
    //     m_Dirty = false;
    // }
}

bool ProjectPanel::DrawMeta(io::Meta& m) {
    bool c = false;
    char nameBuf[128];
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", m.name.c_str());
    char authBuf[128];
    std::snprintf(authBuf, sizeof(authBuf), "%s", m.author.c_str());
    char unitBuf[16];
    std::snprintf(unitBuf, sizeof(unitBuf), "%s", m.units.c_str());

    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
        m.name = nameBuf;
        c = true;
    }
    if (ImGui::InputText("Author", authBuf, sizeof(authBuf))) {
        m.author = authBuf;
        c = true;
    }
    if (ImGui::InputText("Units", unitBuf, sizeof(unitBuf))) {
        m.units = unitBuf;
        c = true;
    }
    return c;
}

bool ProjectPanel::DrawParams(io::ParamsMap& params) {
    bool c = false;
    for (auto& kv : params) {
        const std::string& key = kv.first;
        auto& pv = kv.second;

        ImGui::PushID(key.c_str());
        switch (pv.type) {
            case io::ParamValue::Type::Boolean: {
                bool b = pv.boolean;
                if (ImGui::Checkbox(key.c_str(), &b)) {
                    pv.boolean = b;
                    c = true;
                }
            } break;
            case io::ParamValue::Type::Number: {
                double val = pv.number;
                if (ImGui::InputDouble((key + "##num").c_str(), &val, 1.0, 10.0, "%.3f")) {
                    pv.number = val;
                    c = true;
                }
            } break;
            case io::ParamValue::Type::String: {
                char buf[256];
                std::snprintf(buf, sizeof(buf), "%s", pv.string.c_str());
                if (ImGui::InputText((key + "##str").c_str(), buf, sizeof(buf))) {
                    pv.string = buf;
                    c = true;
                }
            } break;
        }
        ImGui::PopID();
    }
    return c;
}
