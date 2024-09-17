#include "colorAttachment.hpp"

#include "../Models/graphic.hpp"
#include "../Models/graphicPlus.hpp"

vulkanWrapper::colorAttachment::colorAttachment() = default;
vulkanWrapper::colorAttachment::colorAttachment(
    VkFormat format, VkExtent2D extent, uint32_t layerCount,
    VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages) {
  Create(format, extent, layerCount, sampleCount, otherUsages);
}
// Non-const Function
void vulkanWrapper::colorAttachment::Create(VkFormat format, VkExtent2D extent,
                                            uint32_t layerCount,
                                            VkSampleCountFlagBits sampleCount,
                                            VkImageUsageFlags otherUsages) {
  VkImageCreateInfo imageCreateInfo = {
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {extent.width, extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = layerCount,
      .samples = sampleCount,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | otherUsages};
  memory.Create(
      imageCreateInfo,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
          bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) *
              VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
  view.Create(memory.Image(),
              layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                             : VK_IMAGE_VIEW_TYPE_2D,
              format, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount});
}
// Static Function
// 该函数用于检查某一格式的图像可否被用作颜色附件
bool vulkanWrapper::colorAttachment::FormatAvailability(VkFormat format,
                                                        bool supportBlending) {
  return graphic::Plus().FormatProperties(format).optimalTilingFeatures &
         VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT << uint32_t(supportBlending);
}
