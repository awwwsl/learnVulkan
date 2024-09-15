#pragma once

#include "../Utils/VkResultThrowable.hpp"

#include "../Vulkan/commandBuffer.hpp"
#include "../Vulkan/commandPool.hpp"
#include <vulkan/vulkan.h>

class graphicPlusImpl;

class graphicPlus {
  graphicPlus();
  ~graphicPlus();

  graphicPlusImpl *impl;

  // 禁止拷贝和赋值
  graphicPlus(const graphicPlus &) = delete;
  graphicPlus &operator=(const graphicPlus &) = delete;

public:
  // Getter
  const VkFormatProperties &FormatProperties(VkFormat format) const;
  const vulkanWrapper::commandPool &CommandPool_Graphics() const;
  const vulkanWrapper::commandPool &CommandPool_Compute() const;
  const vulkanWrapper::commandBuffer &CommandBuffer_Transfer() const;

  // Const Function
  VkResultThrowable
  ExecuteCommandBuffer_Graphics(VkCommandBuffer commandBuffer) const;

  VkResultThrowable
  ExecuteCommandBuffer_Compute(VkCommandBuffer commandBuffer) const;

  VkResultThrowable AcquireImageOwnership_Presentation(
      VkSemaphore semaphore_renderingIsOver,
      VkSemaphore semaphore_ownershipIsTransfered,
      VkFence fence = VK_NULL_HANDLE) const;

  // 单例
  static graphicPlus &Singleton();
};
