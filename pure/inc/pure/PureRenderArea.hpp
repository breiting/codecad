#pragma once
#include <imgui.h>

#include <glm/vec4.hpp>

namespace pure {

// Computes a viewport rectangle from the current ImGui window's content region.
// Converts to framebuffer pixels using given scale.
class PureRenderArea {
   public:
    // Call inside an ImGui window that should host the 3D view.
    // Returns viewport (x, y, w, h) in framebuffer pixels.
    static ImVec4 ComputeFramebufferViewport(float scaleX, float scaleY) {
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();

        ImVec2 p0 = {winPos.x + contentMin.x, winPos.y + contentMin.y};
        ImVec2 p1 = {winPos.x + contentMax.x, winPos.y + contentMax.y};
        float x = p0.x * scaleX;
        float y = p0.y * scaleY;
        float w = (p1.x - p0.x) * scaleX;
        float h = (p1.y - p0.y) * scaleY;

        return ImVec4(x, y, w, h);
    }
};
}  // namespace pure
