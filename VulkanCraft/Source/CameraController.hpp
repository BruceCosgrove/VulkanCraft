#pragma once

#include <Engine/Core/Timestep.hpp>
#include <Engine/Input/Event/KeyEvents.hpp>
#include <Engine/Input/Event/MouseEvents.hpp>
#include <Engine/Input/Event/WindowEvents.hpp>
#include <glm/glm.hpp>

namespace vc
{
    // TODO: the camera controller will be tied to an entity, which will supply the position.
    class CameraController
    {
    public:
        glm::mat4 GetViewProjection();

        void SetPosition(glm::vec3 position);
        void SetRotation(glm::vec3 rotation);

        void SetFOV(float fovRadians);
        void SetNearPlane(float nearPlane);
        void SetFarPlane(float farPlane);

        void SetMovementSpeed(float movementSpeed);
        void SetMouseSensitivity(float mouseSensitivity);

        void OnUpdate(eng::Timestep timestep);
        void OnEvent(eng::Event& event);
    private:
        void OnKeyPressEvent(eng::KeyPressEvent& event);
        void OnMouseButtonPressEvent(eng::MouseButtonPressEvent& event);
        void OnMouseMoveEvent(eng::MouseMoveEvent& event);
        void OnWindowFramebufferResizeEvent(eng::WindowFramebufferResizeEvent& event);
        void OnWindowFocusEvent(eng::WindowFocusEvent& event);

        void RecalculateView();
        void RecalculateProjection();
    private:
        glm::mat4 m_View; // explicitly uninitialized
        glm::mat4 m_Projection; // explicitly uninitialized

        glm::vec3 m_Position{};
        glm::vec3 m_Rotation{};

        float m_FOV = 1.0f;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1.0f;

        float m_MovementSpeed = 1.0f;
        float m_MouseSensitivity = 1.0f;

        glm::ivec3 m_MovementDirection{};
        glm::vec2 m_LastMousePosition; // explicitly uninitialized
        glm::vec2 m_LastViewportSizeInverse; // explicitly uninitialized

        bool m_RecalculateView = true;
        bool m_RecalculateProjection = true;
        bool m_IgnoreLastMousePosition = true;
        bool m_EnableMouseMovement = false;
    };
}
