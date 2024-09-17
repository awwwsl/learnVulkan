#pragma once

#include "attachment.hpp"

namespace vulkanWrapper {

class colorAttachment : public attachment {
public:
  colorAttachment();
  colorAttachment(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
                  VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                  VkImageUsageFlags otherUsages = 0);
  // Non-const Function
  void Create(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
              VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
              VkImageUsageFlags otherUsages = 0);
  // Static Function
  // 该函数用于检查某一格式的图像可否被用作颜色附件
  static bool FormatAvailability(VkFormat format, bool supportBlending = true);
};

} // namespace vulkanWrapper
