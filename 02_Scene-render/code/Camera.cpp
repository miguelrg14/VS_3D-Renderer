
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "Camera.hpp"
#include <SDL.h>

namespace udit 
{

    void Camera::process_keyboard(const Uint8* keys, float deltaTime)
    {
        // Forward direction
        glm::vec3 forward = glm::normalize(glm::vec3(target - location));
        // Right axis from forward and the world up (0,1,0)
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f)));
        float velocity = movementSpeed * deltaTime;

        if (keys[SDL_SCANCODE_W]) move(forward * velocity);
        if (keys[SDL_SCANCODE_S]) move(-forward * velocity);
        if (keys[SDL_SCANCODE_A]) move(-right * velocity);
        if (keys[SDL_SCANCODE_D]) move(right * velocity);
    }

    void Camera::process_mouse(int xrel, int yrel)
    {
        // Apply the sensitivity
        float xoffset = xrel * mouseSensitivity;
        float yoffset = yrel * mouseSensitivity;

        // 1) Update yaw and pitch
        yaw += xoffset;   // moving the mouse right increases yaw
        pitch -= yoffset;   // moving the mouse up increases pitch

        // 2) Clamp the pitch so the camera can't flip over
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // 3) Rebuild the target from those angles
        updateCameraVectors();
    }

}