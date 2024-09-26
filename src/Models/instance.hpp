#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct alignas(16) instance {
  alignas(16) glm::mat4 model;
  alignas(4) uint32_t textureIndex;
  alignas(4) uint32_t padding[3]; // 12 bytes of padding
};
