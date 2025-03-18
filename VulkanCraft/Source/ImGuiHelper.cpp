#include "ImGuiHelper.hpp"
#include <Engine/Core/DataTypes.hpp>

using namespace eng;

namespace vc
{
    ImGuiHelper::ImGuiHelper()
    {
        // TODO: load and upload font

        // Configuration.

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_IsSRGB;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        style.Colors[ImGuiCol_TitleBgCollapsed].w = 1.0f;

        // NOTE: There is a PR for this on ImGui's repository, but it needn't be that complicated.
        // https://github.com/ocornut/imgui/pull/7826
        // Information I used to understand the issue.
        // https://www.reddit.com/r/vulkan/comments/wbh3ul/vulkan_color_space_is_different_when_using_rgb/
        // Convert to sRGB.
        for (u32 i = 0; i < ImGuiCol_COUNT; i++)
        {
            style.Colors[i].x = std::powf(style.Colors[i].x, 2.2f);
            style.Colors[i].y = std::powf(style.Colors[i].y, 2.2f);
            style.Colors[i].z = std::powf(style.Colors[i].z, 2.2f);
            style.Colors[i].w = std::powf(style.Colors[i].w, 2.2f);
        }
    }

    ImGuiHelper::~ImGuiHelper()
    {

    }
}
