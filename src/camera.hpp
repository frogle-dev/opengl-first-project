#pragma once

#include "../lib/glad.h"
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <algorithm>


// defualt values
const float def_yaw = -90.0f;
const float def_pitch = 0.0f;
const float def_sensitivity = 0.1f;
const float def_fov = 85.0f;


class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 straightFront; // follows yaw movement but not pitch, used for purely horizontal movement
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float sensitivity;
    float fov;

    Camera(glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f), float _yaw = def_yaw, float _pitch = def_pitch)
    {
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        sensitivity = def_sensitivity;
        fov = def_fov;

        position = _position;
        worldUp = _up;
        yaw = _yaw;
        pitch = _pitch;
        UpdateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(
            position,
            position + front,
            up
        );
    }

    void ProcessMouseMovement(float xOffset, float yOffset)
    {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        UpdateCameraVectors();
    }

private:
    void UpdateCameraVectors()
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));

        straightFront = glm::normalize(glm::cross(right, worldUp));
    }
};
