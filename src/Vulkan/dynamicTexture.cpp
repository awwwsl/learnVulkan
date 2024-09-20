#include "dynamicTexture.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

vulkanWrapper::dynamicTexture::dynamicTexture() = default;
vulkanWrapper::dynamicTexture::~dynamicTexture() = default;

void vulkanWrapper::dynamicTexture::CreateImageMemory(
    VkImageType imageType, VkFormat format, VkExtent3D extent,
    uint32_t mipLevelCount, uint32_t arrayLayerCount,
    VkImageCreateFlags flags) {
  VkImageCreateInfo imageCreateInfo = {
      .flags = flags,
      .imageType = imageType,
      .format = format,
      .extent = extent,
      .mipLevels = mipLevelCount,
      .arrayLayers = arrayLayerCount,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
  memory.Create(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
void vulkanWrapper::dynamicTexture::CreateImageViews(
    VkImageViewType viewType, VkFormat format, uint32_t mipLevelCount,
    uint32_t arrayLayerCount, VkImageViewCreateFlags flags) {
  views.resize(arrayLayerCount);
  for (uint32_t i = 0; i < arrayLayerCount; i++) {
    VkImageViewCreateInfo imageViewCreateInfo = {
        .flags = flags,
        .image = memory.Image(),
        .viewType = viewType,
        .format = format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mipLevelCount,
            .baseArrayLayer = i,
            .layerCount = 1,
        }};
    views[i].Create(imageViewCreateInfo);
  }
}
