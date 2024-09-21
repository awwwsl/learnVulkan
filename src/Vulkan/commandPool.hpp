#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"
#include "commandBuffer.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vulkanWrapper {

class commandPool {
  VkCommandPool handle = VK_NULL_HANDLE;

public:
  commandPool();
  inline commandPool(VkCommandPoolCreateInfo &createInfo) {
    Create(createInfo);
  }
  inline commandPool(uint32_t queueFamilyIndex,
                     VkCommandPoolCreateFlags flags = 0) {
    Create(queueFamilyIndex, flags);
  }
  commandPool(commandPool &&other) noexcept;
  ~commandPool();

  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;

  // Const Function

  //  HACK: Copy
  VkResultThrowable AllocateBuffers(
      std::vector<VkCommandBuffer> &buffers,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
  VkResultThrowable AllocateBuffers(
      vulkanWrapper::commandBuffer &buffer,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

  //  HACK: Copy
  void FreeBuffers(std::vector<commandBuffer *> &buffers) const;
  void FreeBuffers(std::vector<VkCommandBuffer> &buffers) const;

  // Non-const Function
  VkResultThrowable Create(VkCommandPoolCreateInfo &createInfo);
  inline VkResultThrowable Create(uint32_t queueFamilyIndex,
                                  VkCommandPoolCreateFlags flags = 0) {
    VkCommandPoolCreateInfo createInfo = {.flags = flags,
                                          .queueFamilyIndex = queueFamilyIndex};
    return Create(createInfo);
  }
};

} // namespace vulkanWrapper
