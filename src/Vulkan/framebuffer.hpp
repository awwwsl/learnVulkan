#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class framebuffer {
  VkFramebuffer handle = VK_NULL_HANDLE;
  VkExtent2D size = {0, 0};

public:
  framebuffer();
  inline framebuffer(VkFramebufferCreateInfo &createInfo) {
    Create(createInfo);
  }
  framebuffer(framebuffer &&other) noexcept;
  ~framebuffer();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  inline VkExtent2D Size() const { return size; }
  // Non-const Function
  VkResultThrowable Create(VkFramebufferCreateInfo &createInfo);
};

} // namespace vulkanWrapper
