#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class entity {
public:
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation; // xyz

  inline entity(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation)
      : position(position), scale(scale), rotation(rotation) {}

  inline entity(glm::vec3 position)
      : entity(position, defaultScale, defaultRotation) {}

  static const constexpr glm::vec3 defaultScale = {1.0f, 1.0f, 1.0f};
  static const constexpr glm::vec3 defaultRotation = {0.0f, 0.0f, 0.0f};

  inline glm::mat4 getModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    model = glm::rotate(model, glm::radians(rotation.x),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z),
                        glm::vec3(0.0f, 0.0f, 1.0f));

    return model;
  }
};
