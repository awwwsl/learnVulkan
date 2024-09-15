#pragma once

#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "graphic.hpp"
#include "graphicPlus.hpp"

#include <vector>

class graphicPlusImpl {
  friend class graphicPlus;

  std::vector<VkFormatProperties> formatProperties;
  vulkanWrapper::commandPool commandPool_graphics;
  vulkanWrapper::commandPool commandPool_presentation;
  vulkanWrapper::commandPool commandPool_compute;
  vulkanWrapper::commandBuffer
      commandBuffer_transfer; // 从commandPool_graphics分配
  vulkanWrapper::commandBuffer commandBuffer_presentation;
  graphicPlusImpl(graphicPlusImpl &&) = delete;
  graphicPlusImpl();
  ~graphicPlusImpl();

public:
  inline static graphicPlusImpl &Singleton() {
    static graphicPlusImpl instance;
    return instance;
  }

  VkResultThrowable
  ExecuteCommandBuffer_Graphics(VkCommandBuffer commandBuffer) const;
  VkResultThrowable
  ExecuteCommandBuffer_Compute(VkCommandBuffer commandBuffer) const;
  VkResultThrowable AcquireImageOwnership_Presentation(
      VkSemaphore semaphore_renderingIsOver,
      VkSemaphore semaphore_ownershipIsTransfered,
      VkFence fence = VK_NULL_HANDLE) const;

  // Getter
  inline const VkFormatProperties &FormatProperties(VkFormat format) const {
    return formatProperties[format];
  }

  inline const std::vector<VkFormatProperties> FormatProperties() const {
    return formatProperties;
  }
};
