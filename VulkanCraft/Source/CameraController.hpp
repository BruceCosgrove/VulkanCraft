#pragma once

#include <Engine.hpp>

using namespace eng;

namespace vc
{
    // TODO: the camera controller will be tied to an entity, which will supply the position.
    class CameraController
    {
    public:
        mat4 GetViewProjection();

        void SetPosition(vec3 position);
        void SetRotation(vec3 rotation);

        void SetFOV(f32 fovRadians);
        void SetNearPlane(f32 nearPlane);
        void SetFarPlane(f32 farPlane);

        void SetMovementSpeed(f32 movementSpeed);
        void SetMouseSensitivity(f32 mouseSensitivity);

        void OnUpdate(Timestep timestep);
        void OnEvent(Event& event);
    private:
        void OnKeyPressEvent(KeyPressEvent& event);
        void OnMouseButtonPressEvent(MouseButtonPressEvent& event);
        void OnMouseMoveEvent(MouseMoveEvent& event);
        void OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event);
        void OnWindowFocusEvent(WindowFocusEvent& event);

        void RecalculateView();
        void RecalculateProjection();
    private:
        mat4 m_View; // explicitly uninitialized
        mat4 m_Projection; // explicitly uninitialized

        vec3 m_Position{};
        vec3 m_Rotation{};

        f32 m_FOV = 1.0f;
        f32 m_NearPlane = 0.1f;
        f32 m_FarPlane = 1.0f;

        f32 m_MovementSpeed = 1.0f;
        f32 m_MouseSensitivity = 1.0f;

        ivec3 m_MovementDirection{};
        vec2 m_LastMousePosition; // explicitly uninitialized
        vec2 m_LastViewportSizeInverse; // explicitly uninitialized

        bool m_RecalculateView = true;
        bool m_RecalculateProjection = true;
        bool m_IgnoreLastMousePosition = true;
        bool m_EnableMouseMovement = false;
    };
}
