#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace vulkanWrapper {

class gameTexture {
  uint64_t index;

public:
  gameTexture(uint64_t index) : index(index) {}
  VkImageView view();

  void updateView();
};

} // namespace vulkanWrapper
