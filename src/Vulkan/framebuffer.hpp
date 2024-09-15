#pragma once

#define GLFW_INCLUDE_VULKAN

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class framebuffer {
  VkFramebuffer handle = VK_NULL_HANDLE;

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
  // Non-const Function
  VkResultThrowable Create(VkFramebufferCreateInfo &createInfo);
};

} // namespace vulkanWrapper
