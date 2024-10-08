#include "commandPool.hpp"

#include "../Models/graphic.hpp"

#include <stdio.h>
#include <vector>
#include <vulkan/vulkan_core.h>

vulkanWrapper::commandPool::commandPool() {}

VkResultThrowable
vulkanWrapper::commandPool::Create(VkCommandPoolCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  VkResult result = vkCreateCommandPool(graphic::Singleton().Device(),
                                        &createInfo, nullptr, &handle);
  if (result)
    printf("[ commandPool ] ERROR: Failed to create a "
           "command pool!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  printf("[ commandPool ] DEBUG: CommandPool created with handle %p\n", handle);
#endif
  return result;
}

VkResultThrowable vulkanWrapper::commandPool::AllocateBuffers(
    std::vector<VkCommandBuffer> &buffers, VkCommandBufferLevel level) const {
  VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = handle,
      .level = level,
      .commandBufferCount = uint32_t(buffers.size())};
  VkResult result = vkAllocateCommandBuffers(graphic::Singleton().Device(),
                                             &allocateInfo, buffers.data());
  if (result)
    printf("[ commandPool ] ERROR: Failed to allocate "
           "command buffers!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  printf("[ commandPool ] DEBUG: Allocated %d command buffers with handles: ",
         allocateInfo.commandBufferCount);
  for (int i = 0; i < allocateInfo.commandBufferCount; i++)
    if (i == allocateInfo.commandBufferCount - 1)
      printf("[%d]%p\n", i, buffers[i]);
    else
      printf("[%d]%p, ", i, buffers[i]);
#endif
  return result;
}
VkResultThrowable vulkanWrapper::commandPool::AllocateBuffers(
    vulkanWrapper::commandBuffer &buffer, VkCommandBufferLevel level) const {
  std::vector<VkCommandBuffer> buffers(1);
  buffers[0] = buffer;
  VkResultThrowable result = AllocateBuffers(buffers, level);
  if (!result)
    buffer.handle = buffers[0];
  return result;
}

void vulkanWrapper::commandPool::FreeBuffers(
    std::vector<VkCommandBuffer> &buffers) const {
  vkFreeCommandBuffers(graphic::Singleton().Device(), handle, buffers.size(),
                       buffers.data());
  buffers.clear();
}
//  HACK: Copy
void vulkanWrapper::commandPool::FreeBuffers(
    std::vector<commandBuffer *> &buffers) const {
  std::vector<VkCommandBuffer> bufferHandles(buffers.size());
  for (size_t i = 0; i < buffers.size(); i++) {
    bufferHandles[i] = buffers[i]->handle;
  }

  return FreeBuffers(bufferHandles);
}

vulkanWrapper::commandPool::commandPool(commandPool &&other) noexcept {
  MoveHandle;
}

vulkanWrapper::commandPool::~commandPool() {
  DestroyHandleBy(vkDestroyCommandPool, "commandPool");
}
