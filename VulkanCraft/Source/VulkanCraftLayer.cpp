#include "VulkanCraftLayer.hpp"
#include <imgui.h>

namespace vc
{
    void VulkanCraftLayer::OnAttach()
    {

    }

    void VulkanCraftLayer::OnDetach()
    {

    }

    void VulkanCraftLayer::OnEvent(eng::Event& event)
    {
        ENG_LOG_INFO("VulkanCraftLayer::OnEvent(TODO: event logging)");
    }

    void VulkanCraftLayer::OnUpdate(eng::Timestep timestep)
    {

    }

    void VulkanCraftLayer::OnRender()
    {

    }

    void VulkanCraftLayer::OnImGuiRender()
    {
        UI_Dockspace();

        ImGui::ShowDemoWindow();

        ImGui::Begin("test window");
        ImGui::Button("test button");
        ImGui::End();
    }

    void VulkanCraftLayer::UI_Dockspace()
    {
        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus /* |
            ImGuiWindowFlags_MenuBar */;

        ImGuiViewport& viewport = *ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport.WorkPos);
        ImGui::SetNextWindowSize(viewport.WorkSize);
        ImGui::SetNextWindowViewport(viewport.ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dockspace", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGui::DockSpace(ImGui::GetID("Dockspace"));

        // Where a menubar would go, were one to be wanted.

        ImGui::End();
    }
}
