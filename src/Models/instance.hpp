#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct alignas(16) instance {
  alignas(16) glm::mat4 model;
  alignas(4) uint32_t textureIndex;
  alignas(4) bool isValid;
  alignas(4) uint32_t padding[2];
};
