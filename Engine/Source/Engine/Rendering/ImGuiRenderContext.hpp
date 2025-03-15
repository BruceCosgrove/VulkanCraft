#pragma once

#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Input/Event/Event.hpp"
#include <vulkan/vulkan.h>

namespace eng
{
    class Window;

    class ImGuiRenderContext
    {
    public:
        ENG_IMMOVABLE_UNCOPYABLE_CLASS(ImGuiRenderContext);

        ImGuiRenderContext(Window& window, VkRenderPass renderPass);
        ~ImGuiRenderContext();

        void BeginFrame();
        void EndFrame(VkCommandBuffer commandBuffer);
        void RenderMultiViewportWindows();

        void OnEvent(Event& event);
    };
}
