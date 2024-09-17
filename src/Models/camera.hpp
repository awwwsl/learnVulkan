#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdint.h>
#include <sys/types.h>

class camera {
public:
  glm::vec3 position;
  // glm::vec3 speed;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;

  // degree
  float yaw;
  float pitch;
  float fov;

  float nearPlane;
  float farPlane;

  static const constexpr glm::vec3 defaultPosition = {0.0f, 0.0f, 0.0f};
  static const constexpr glm::vec3 defaultFront = {0.0f, 0.0f, -1.0f};
  static const constexpr glm::vec3 defaultUp = {0.0f, 1.0f, 0.0f};
  static const constexpr glm::vec3 defaultRight = {1.0f, 0.0f, 0.0f};

  static const constexpr float defaultYaw = -90.0f;
  static const constexpr float defaultPitch = 0.0f;
  static const constexpr float defaultFov = 90.0f;

  static const constexpr float defaultNearPlane = 0.1f;
  static const constexpr float defaultFarPlane = 100.0f;

  inline camera()
      : camera(defaultPosition, defaultFront, defaultUp, defaultRight,
               defaultYaw, defaultPitch, defaultFov, defaultNearPlane,
               defaultFarPlane) {}

  inline camera(glm::vec3 position, glm::vec3 front, glm::vec3 up,
                glm::vec3 right, float yaw, float pitch, float fov,
                float nearPlane, float farPlane)
      : position(position), front(front), up(up), right(right), yaw(yaw),
        pitch(pitch), fov(fov), nearPlane(nearPlane), farPlane(farPlane) {}

  // 获取视图矩阵
  inline glm::mat4 getViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  // 获取投影矩阵
  inline glm::mat4 getProjectionMatrix(float aspectRatio) {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
  }

  inline glm::mat4 getProjectionMatrix(u_int16_t width, u_int16_t height) {
    return getProjectionMatrix((float)width / (float)height);
  }

  // 获取MVP矩阵
  inline glm::mat4 getMVPMatrix(glm::mat4 model, float aspectRatio) {
    glm::mat4 view = getViewMatrix();
    glm::mat4 projection = getProjectionMatrix(aspectRatio);
    return projection * view * model;
  }
};
