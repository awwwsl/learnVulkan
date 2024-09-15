#include "commandBuffer.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

vulkanWrapper::commandBuffer::commandBuffer() = default;

VkResultThrowable vulkanWrapper::commandBuffer::Begin(
    VkCommandBufferUsageFlags usageFlags,
    VkCommandBufferInheritanceInfo &inheritanceInfo) const {
  inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = usageFlags,
      .pInheritanceInfo = &inheritanceInfo};
  VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
  if (result)
    printf("[ commandBuffer ] ERROR: Failed to begin a "
           "command buffer!\nError code: %d\n",
           int32_t(result));
  return result;
}

VkResultThrowable vulkanWrapper::commandBuffer::Begin(
    VkCommandBufferUsageFlags usageFlags) const {
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = usageFlags,
  };
  VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
  if (result)
    printf("[ commandBuffer ] ERROR: Failed to begin a "
           "command buffer!\nError code: %d\n",
           int32_t(result));
  return result;
}

VkResultThrowable vulkanWrapper::commandBuffer::End() const {
  VkResult result = vkEndCommandBuffer(handle);
  if (result)
    printf("[ commandBuffer ] ERROR: Failed to end a "
           "command buffer!\nError code: %d\n",
           int32_t(result));
  return result;
};

vulkanWrapper::commandBuffer::commandBuffer(commandBuffer &&other) noexcept {
  MoveHandle;
}
