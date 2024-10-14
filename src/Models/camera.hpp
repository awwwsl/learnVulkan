#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>

#include <array>

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

  float aspectRatio;

  // static const constexpr glm::vec3 defaultPosition = {0.0f, 0.0f, 0.0f};
  static const constexpr glm::vec3 defaultPosition = {-48.0f, -3.0f, -48.0f};

  static const constexpr glm::vec3 defaultFront = {0.0f, 0.0f, -1.0f};
  static const constexpr glm::vec3 defaultUp = {0.0f, 1.0f, 0.0f};
  static const constexpr glm::vec3 defaultRight = {1.0f, 0.0f, 0.0f};

  static const constexpr float defaultYaw = -90.0f;
  static const constexpr float defaultPitch = 0.0f;
  static const constexpr float defaultFov = 90.0f;

  static const constexpr float defaultNearPlane = 0.02f;
  static const constexpr float defaultFarPlane = 1000.0f;

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
  inline glm::mat4 getProjectionMatrix() {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
  }

  // 获取MVP矩阵
  inline glm::mat4 getMVPMatrix(glm::mat4 model) {
    glm::mat4 view = getViewMatrix();
    glm::mat4 projection = getProjectionMatrix();
    return projection * view * model;
  }

  inline std::array<glm::vec4, 6> frustumPlanes() {
    glm::mat4 view = getViewMatrix();
    glm::mat4 projection = getProjectionMatrix();
    glm::mat4 pv = projection * view;

    std::array<glm::vec4, 6> planes;

    // Left plane
    planes[0] = glm::vec4(pv[0][3] + pv[0][0], pv[1][3] + pv[1][0],
                          pv[2][3] + pv[2][0], pv[3][3] + pv[3][0]);
    // Right plane
    planes[1] = glm::vec4(pv[0][3] - pv[0][0], pv[1][3] - pv[1][0],
                          pv[2][3] - pv[2][0], pv[3][3] - pv[3][0]);
    // Bottom plane
    planes[2] = glm::vec4(pv[0][3] + pv[0][1], pv[1][3] + pv[1][1],
                          pv[2][3] + pv[2][1], pv[3][3] + pv[3][1]);
    // Top plane
    planes[3] = glm::vec4(pv[0][3] - pv[0][1], pv[1][3] - pv[1][1],
                          pv[2][3] - pv[2][1], pv[3][3] - pv[3][1]);
    // Near plane
    planes[4] = glm::vec4(pv[0][3] + pv[0][2], pv[1][3] + pv[1][2],
                          pv[2][3] + pv[2][2], pv[3][3] + pv[3][2]);
    // Far plane
    planes[5] = glm::vec4(pv[0][3] - pv[0][2], pv[1][3] - pv[1][2],
                          pv[2][3] - pv[2][2], pv[3][3] - pv[3][2]);

    for (auto &plane : planes) {
      float length = glm::length(glm::vec3(plane.x, plane.y, plane.z));
      plane /= length;
    }

    return planes;
  }

  inline void forward(float distance) { position += front * distance; }
  inline void rightward(float distance) { position += right * distance; }
  inline void upward(float distance) { position += up * distance; }

  inline void horizentalForward(float distance) {
    position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * distance;
  }
  inline void horizentalRightward(float distance) {
    position += glm::normalize(glm::vec3(right.x, 0.0f, right.z)) * distance;
  }
  inline void verticalUpward(float distance) {
    position -= defaultUp * distance;
  }

  inline void updateCameraVectors() {
    front = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                      sin(glm::radians(pitch)),
                      sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, defaultUp));
    up = glm::normalize(glm::cross(right, front));
  }

  inline static camera &Singleton() {
    static camera instance;
    return instance;
  }
};
