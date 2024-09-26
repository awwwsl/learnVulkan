#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class block {
private:
  uint8_t facing;

public:
  glm::ivec3 position;
  glm::ivec3 scale;
  // 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z

  void setFacing(uint8_t facing) { this->facing = facing % 6; }

  inline block(glm::ivec3 position, glm::ivec3 scale, uint8_t facing)
      : facing(facing), position(position), scale(scale) {}

  inline block(glm::vec3 position)
      : block(position, defaultScale, defaultFacing) {}

  static const constexpr glm::ivec3 defaultScale = {1, 1, 1};
  static const constexpr uint8_t defaultFacing = 0;

  inline glm::mat4 getModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(position));
    model = glm::scale(model, glm::vec3(scale));

    switch (facing % 6) {
    case 0:
      // model = DoNothing;
      break;
    case 1:
      model =
          glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      break;
    case 2:
      model =
          glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      break;
    case 3:
      model =
          glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      break;
    case 4:
      model =
          glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    case 5:
      model =
          glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      break;
    }

    return model;
  }
};
