#pragma once

// Include everything* under Engine/

#include "Engine/Core/Application.hpp"
#include "Engine/Core/AssertOrVerify.hpp"
#include "Engine/Core/Attributes.hpp"
#include "Engine/Core/ClassTypes.hpp"
#include "Engine/Core/DataTypes.hpp"
#include "Engine/Core/DebugBreak.hpp"
#include "Engine/Core/Enums.hpp"
#include "Engine/Core/FunctionBindings.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Core/Timestep.hpp"
#include "Engine/Input/Keycode.hpp"
#include "Engine/Input/Layer.hpp"
#include "Engine/Input/Modifiers.hpp"
#include "Engine/Input/MouseButtons.hpp"
#include "Engine/Input/Window.hpp"
#include "Engine/Input/Event/Event.hpp"
#include "Engine/Input/Event/KeyEvents.hpp"
#include "Engine/Input/Event/MouseEvents.hpp"
#include "Engine/Input/Event/WindowEvents.hpp"
#include "Engine/IO/FileIO.hpp"
#include "Engine/Rendering/BufferUtils.hpp"
#include "Engine/Rendering/Framebuffer.hpp"
#include "Engine/Rendering/FramebufferAttachment.hpp"
#include "Engine/Rendering/Image.hpp"
#include "Engine/Rendering/ImageUtils.hpp"
#include "Engine/Rendering/LocalTexture.hpp"
#include "Engine/Rendering/RenderContext.hpp"
#include "Engine/Rendering/RenderPass.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/StorageBuffer.hpp"
#include "Engine/Rendering/Texture2D.hpp"
#include "Engine/Rendering/Texture2DArray.hpp"
#include "Engine/Rendering/UniformBuffer.hpp"
#include "Engine/Rendering/VertexBuffer.hpp"
#include "Engine/Threading/DynamicResource.hpp"

// Include glm

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
