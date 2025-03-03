#include "CameraController.hpp"
#include <Engine/Core/Attributes.hpp>
#include <Engine/Core/Log.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace vc
{
    glm::mat4 CameraController::GetViewProjection()
    {
        // Defer recalculating the matricies until requested.
        if (m_RecalculateView)
            RecalculateView();
        if (m_RecalculateProjection)
            RecalculateProjection();
        return m_Projection * m_View;
    }

    void CameraController::SetPosition(glm::vec3 position)
    {
        m_Position = position;
        m_RecalculateView = true;
    }

    void CameraController::SetRotation(glm::vec3 rotation)
    {
        m_Rotation = rotation;
        m_RecalculateView = true;
    }

    void CameraController::SetFOV(float fovRadians)
    {
        m_FOV = fovRadians;
        m_RecalculateProjection = true;
    }

    void CameraController::SetNearPlane(float nearPlane)
    {
        m_NearPlane = nearPlane;
        m_RecalculateProjection = true;
    }

    void CameraController::SetFarPlane(float farPlane)
    {
        m_FarPlane = farPlane;
        m_RecalculateProjection = true;
    }

    void CameraController::SetMovementSpeed(float movementSpeed)
    {
        m_MovementSpeed = movementSpeed;
    }

    void CameraController::SetMouseSensitivity(float mouseSensitivity)
    {
        m_MouseSensitivity = mouseSensitivity;
    }

    void CameraController::OnUpdate(eng::Timestep timestep)
    {
        if (m_MovementDirection != glm::ivec3())
        {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 movementDirection = rotation * glm::vec4(m_MovementDirection, 1.0f);
            m_Position += movementDirection * (m_MovementSpeed * timestep);
            m_RecalculateView = true;
        }
    }

    void CameraController::OnEvent(eng::Event& event)
    {
        event.Dispatch(this, &CameraController::OnKeyPressEvent);
        event.Dispatch(this, &CameraController::OnMouseButtonPressEvent);
        event.Dispatch(this, &CameraController::OnMouseMoveEvent);
        event.Dispatch(this, &CameraController::OnWindowFramebufferResizeEvent);
        event.Dispatch(this, &CameraController::OnWindowFocusEvent);
    }

    void CameraController::OnKeyPressEvent(eng::KeyPressEvent& event)
    {
        std::int32_t direction = event.IsPressed() ? 1 : -1;
        switch (event.GetKeycode())
        {
            case eng::Keycode::A: // -x
                direction = -direction;
                ENG_FALLTHROUGH;
            case eng::Keycode::D: // +x
                m_MovementDirection.x += direction;
                break;
            case eng::Keycode::LeftControl: // -y
                direction = -direction;
                ENG_FALLTHROUGH;
            case eng::Keycode::Space: // +y
                m_MovementDirection.y += direction;
                break;
            case eng::Keycode::W: // -z
                direction = -direction;
                ENG_FALLTHROUGH;
            case eng::Keycode::S: // +z
                m_MovementDirection.z += direction;
                break;
        }

        //ENG_LOG_DEBUG("CameraController::m_MovementDirection = <{},{},{}>", m_MovementDirection.x, m_MovementDirection.y, m_MovementDirection.z);
    }

    void CameraController::OnMouseButtonPressEvent(eng::MouseButtonPressEvent& event)
    {
        m_EnableMouseMovement = event.IsPressed();
    }

    void CameraController::OnMouseMoveEvent(eng::MouseMoveEvent& event)
    {
        glm::vec2 mousePosition(event.GetX(), event.GetY());

        if (m_EnableMouseMovement and !m_IgnoreLastMousePosition)
        {
            glm::vec2 deltaMousePosition = mousePosition - m_LastMousePosition;
            glm::vec2 deltaMouseMovement = deltaMousePosition * m_LastViewportSizeInverse * m_MouseSensitivity;
            m_Rotation.y -= deltaMouseMovement.x;
            m_Rotation.x -= deltaMouseMovement.y;
            m_RecalculateView = true;
        }
        else // This current position will be ignored, but not the ones after.
            m_IgnoreLastMousePosition = false;
        // Always set the last mouse position.
        m_LastMousePosition = mousePosition;
    }

    void CameraController::OnWindowFramebufferResizeEvent(eng::WindowFramebufferResizeEvent& event)
    {
        m_LastViewportSizeInverse = 1.0f / glm::vec2(event.GetFramebufferWidth(), event.GetFramebufferHeight());
        m_RecalculateProjection = true;
    }

    void CameraController::OnWindowFocusEvent(eng::WindowFocusEvent& event)
    {
        if (!event.IsFocused())
            m_IgnoreLastMousePosition = true;
    }

    void CameraController::RecalculateView()
    {
        m_View = glm::inverse(glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(glm::quat(m_Rotation)));
        m_RecalculateView = false;
    }

    void CameraController::RecalculateProjection()
    {
        float aspectRatio = static_cast<float>(m_LastViewportSizeInverse.y) / m_LastViewportSizeInverse.x;
        m_Projection = glm::perspective(m_FOV, aspectRatio, m_NearPlane, m_FarPlane);
        m_RecalculateProjection = false;
    }
}
