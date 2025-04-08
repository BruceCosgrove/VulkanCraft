#include "CameraController.hpp"

namespace vc
{
    mat4 CameraController::GetViewProjection()
    {
        // Defer recalculating the matricies until requested.
        if (m_RecalculateView)
            RecalculateView();
        if (m_RecalculateProjection)
            RecalculateProjection();
        return m_Projection * m_View;
    }

    void CameraController::SetPosition(vec3 position)
    {
        m_Position = position;
        m_RecalculateView = true;
    }

    void CameraController::SetRotation(vec3 rotation)
    {
        m_Rotation = rotation;
        m_RecalculateView = true;
    }

    void CameraController::SetFOV(f32 fovRadians)
    {
        m_FOV = fovRadians;
        m_RecalculateProjection = true;
    }

    void CameraController::SetNearPlane(f32 nearPlane)
    {
        m_NearPlane = nearPlane;
        m_RecalculateProjection = true;
    }

    void CameraController::SetFarPlane(f32 farPlane)
    {
        m_FarPlane = farPlane;
        m_RecalculateProjection = true;
    }

    void CameraController::SetMovementSpeed(f32 movementSpeed)
    {
        m_MovementSpeed = movementSpeed;
    }

    void CameraController::SetMouseSensitivity(f32 mouseSensitivity)
    {
        m_MouseSensitivity = mouseSensitivity;
    }

    void CameraController::OnUpdate(Timestep timestep)
    {
        if (m_MovementDirection != ivec3())
        {
            mat4 rotation = glm::rotate(mat4(1.0f), m_Rotation.y, vec3(0.0f, 1.0f, 0.0f));
            vec3 movementDirection = rotation * vec4(m_MovementDirection, 1.0f);
            m_Position += movementDirection * (m_MovementSpeed * timestep.Seconds());
            m_RecalculateView = true;

            // TODO: I have no words to express how many hours of pain this has caused me.
            // For some reason, in release and dist builds, if m_Position isn't used after
            // its value is calculated, some kind of abhorrent msvc "optimization" seems to
            // significantly reduce the precision of the result. From my experimentation,
            // it seems to have roughly 3 bits of exponent.
            // Direct results:
            //  -   Moving up or down is so insanely slow.
            //  -   Moving any direction in the xz-plane is massively subjected to the minimal precision,
            //      to the point where after moving roughly 4 units in either direction, you can't move anymore.
            //
            // FURTHERMORE, this "if used" check seems to be made at RUNTIME, so the variable
            // HAS to be used, not just some mock to make the compiler not optimize it as much.
            // Seriously, after I did this, I went and disabled logging trace messages for
            // release and dist builds, and the bug started happening again.
            //
            // ALSO, this exact bug has happened before using std::fmod or some similar math function.
            // So it seems to be compiler intrinsics completely botching the calculations.
            //
            // This solution is the easist thing that reliably fixes the issue, but is not ideal.
#if not ENG_CONFIG_DEBUG
            // TODO: Side note, apparently spdlog doesn't care if its argument count is more than it needs.
            // Consider using another logging library that has compile-time argument checking.
            ENG_LOG_TRACE("", m_Position.x);
#endif
        }
    }

    void CameraController::OnEvent(Event& event)
    {
        event.Dispatch(this, &CameraController::OnKeyPressEvent);
        event.Dispatch(this, &CameraController::OnMouseButtonPressEvent);
        event.Dispatch(this, &CameraController::OnMouseMoveEvent);
        event.Dispatch(this, &CameraController::OnWindowFramebufferResizeEvent);
        event.Dispatch(this, &CameraController::OnWindowFocusEvent);
    }

    void CameraController::OnKeyPressEvent(KeyPressEvent& event)
    {
        i32 direction = event.IsPressed() ? 1 : -1;
        switch (+event.GetKeycode())
        {
            case Keycode::A: // -x
                direction = -direction;
                ENG_FALLTHROUGH;
            case Keycode::D: // +x
                m_MovementDirection.x += direction;
                break;
            case Keycode::LeftControl: // -y
                direction = -direction;
                ENG_FALLTHROUGH;
            case Keycode::Space: // +y
                m_MovementDirection.y += direction;
                break;
            case Keycode::W: // -z
                direction = -direction;
                ENG_FALLTHROUGH;
            case Keycode::S: // +z
                m_MovementDirection.z += direction;
                break;
        }

        //ENG_LOG_DEBUG("CameraController::m_MovementDirection = <{},{},{}>", m_MovementDirection.x, m_MovementDirection.y, m_MovementDirection.z);
    }

    void CameraController::OnMouseButtonPressEvent(MouseButtonPressEvent& event)
    {
        m_EnableMouseMovement = event.IsPressed() and event.GetButton() == MouseButton::Left;
    }

    void CameraController::OnMouseMoveEvent(MouseMoveEvent& event)
    {
        vec2 mousePosition(event.GetX(), event.GetY());

        if (not m_IgnoreLastMousePosition)
        {
            if (m_EnableMouseMovement)
            {
                vec2 deltaMousePosition = mousePosition - m_LastMousePosition;
                vec2 deltaMouseMovement = deltaMousePosition * m_LastViewportSizeInverse * m_MouseSensitivity;
                m_Rotation.y -= deltaMouseMovement.x;
                m_Rotation.x -= deltaMouseMovement.y;
                m_RecalculateView = true;
            }
        }
        else // This current position will be ignored, but not the ones after.
            m_IgnoreLastMousePosition = false;
        // Always set the last mouse position.
        m_LastMousePosition = mousePosition;
    }

    void CameraController::OnWindowFramebufferResizeEvent(WindowFramebufferResizeEvent& event)
    {
        m_LastViewportSizeInverse = 1.0f / vec2(event.GetFramebufferWidth(), event.GetFramebufferHeight());
        m_RecalculateProjection = true;
    }

    void CameraController::OnWindowFocusEvent(WindowFocusEvent& event)
    {
        if (!event.IsFocused())
            m_IgnoreLastMousePosition = true;
    }

    void CameraController::RecalculateView()
    {
        m_View = glm::inverse(glm::translate(mat4(1.0f), m_Position) * glm::toMat4(quat(m_Rotation)));
        m_RecalculateView = false;
    }

    void CameraController::RecalculateProjection()
    {
        f32 aspectRatio = static_cast<f32>(m_LastViewportSizeInverse.y) / m_LastViewportSizeInverse.x;
        m_Projection = glm::perspective(m_FOV, aspectRatio, m_NearPlane, m_FarPlane);
        m_RecalculateProjection = false;
    }
}
