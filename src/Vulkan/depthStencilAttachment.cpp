#include "depthStencilAttachment.hpp"

#include "../Models/graphic.hpp"
#include "../Models/graphicPlus.hpp"

vulkanWrapper::depthStencilAttachment::depthStencilAttachment() = default;
vulkanWrapper::depthStencilAttachment::depthStencilAttachment(
    VkFormat format, VkExtent2D extent, uint32_t layerCount,
    VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages,
    bool stencilOnly) {
  Create(format, extent, layerCount, sampleCount, otherUsages, stencilOnly);
}
// Non-const Function
void vulkanWrapper::depthStencilAttachment::Create(
    VkFormat format, VkExtent2D extent, uint32_t layerCount,
    VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages,
    bool stencilOnly) {
  VkImageCreateInfo imageCreateInfo = {
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {extent.width, extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = layerCount,
      .samples = sampleCount,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | otherUsages};
  memory.Create(
      imageCreateInfo,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
          bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) *
              VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
  // 确定aspcet mask-------------------------
  VkImageAspectFlags aspectMask = (!stencilOnly) * VK_IMAGE_ASPECT_DEPTH_BIT;
  if (format > VK_FORMAT_S8_UINT)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  else if (format == VK_FORMAT_S8_UINT)
    aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
  //----------------------------------------
  view.Create(memory.Image(),
              layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                             : VK_IMAGE_VIEW_TYPE_2D,
              format, {aspectMask, 0, 1, 0, layerCount});
}
// Static Function
// 该函数用于检查某一格式的图像可否被用作深度模板附件
bool FormatAvailability(VkFormat format) {
  return graphic::Plus().FormatProperties(format).optimalTilingFeatures &
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}
