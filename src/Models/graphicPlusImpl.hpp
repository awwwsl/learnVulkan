#pragma once

#include "../Vulkan//formatInfo.hpp"
#include "../Vulkan/commandBuffer.hpp"
#include "../Vulkan/commandPool.hpp"

#include "graphic.hpp"
#include "graphicPlus.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

class graphicPlusImpl {
  friend class graphicPlus;

  std::vector<VkFormatProperties> formatProperties =
      std::vector<VkFormatProperties>(formatInfo::FormatInfoCount());
  vulkanWrapper::commandPool commandPool_graphics;
  vulkanWrapper::commandPool commandPool_presentation;
  vulkanWrapper::commandPool commandPool_compute;
  vulkanWrapper::commandBuffer
      commandBuffer_transfer; // 从commandPool_graphics分配
  vulkanWrapper::commandBuffer commandBuffer_presentation;
  graphicPlusImpl(graphicPlusImpl &&) = delete;
  graphicPlusImpl();
  ~graphicPlusImpl();

  std::function<void()> CleanUpLambda;

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

  inline const void CleanUp() const { CleanUpLambda(); }
};
