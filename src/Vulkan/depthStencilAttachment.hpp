#pragma once

#include "attachment.hpp"

namespace vulkanWrapper {

class depthStencilAttachment : public attachment {
public:
  depthStencilAttachment();
  depthStencilAttachment(
      VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
      VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
      VkImageUsageFlags otherUsages = 0, bool stencilOnly = false);
  // Non-const Function
  void Create(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
              VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
              VkImageUsageFlags otherUsages = 0, bool stencilOnly = false);
  // Static Function
  // 该函数用于检查某一格式的图像可否被用作深度模板附件
  static bool FormatAvailability(VkFormat format);
};

} // namespace vulkanWrapper
