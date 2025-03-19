#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    class ImGuiRenderContext
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ImGuiRenderContext);

        ImGuiRenderContext(Window& window, VkRenderPass renderPass);
        ~ImGuiRenderContext();

        void BeginFrame(Timestep timestep);
        void EndFrame(VkCommandBuffer commandBuffer);

        void OnEvent(Event& event);
    };
}
