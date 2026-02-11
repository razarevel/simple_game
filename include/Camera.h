#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>

enum CameraMovement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

struct Camera {
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;
  float MovementSpeed;
  float MouseSensitivit;
  float Zoom;

  Camera(glm::vec3 position = glm::vec3(0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH)
      : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
        MouseSensitivit(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraView();
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
  }

  void ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
      Position += Front * velocity;
    if (direction == BACKWARD)
      Position -= Front * velocity;
    if (direction == LEFT)
      Position -= Right * velocity;
    if (direction == RIGHT)
      Position += Right * velocity;
  }

  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constraintPitch = true) {
    std::cout << xoffset << ", " << yoffset << std::endl;
    xoffset *= MouseSensitivit;
    yoffset *= MouseSensitivit;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constraintPitch) {
      if (Pitch > 80.0f)
        Pitch = 80.0f;
      if (Pitch < -80.0f)
        Pitch = -80.0f;
    }

    updateCameraView();
  }

  void ProcessMouseScroll(float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 45.0f)
      Zoom = 45.0f;
  }

private:
  void updateCameraView() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};
