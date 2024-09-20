#define STB_IMAGE_IMPLEMENTATION
#include "texture.hpp"

#include "../Models/graphicPlus.hpp"

#include <stb/stb_image.h>

vulkanWrapper::texture::texture() = default;
vulkanWrapper::texture::~texture() = default;
void vulkanWrapper::texture::CreateImageMemory(VkImageType imageType,
                                               VkFormat format,
                                               VkExtent3D extent,
                                               uint32_t mipLevelCount,
                                               uint32_t arrayLayerCount,
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
void vulkanWrapper::texture::CreateImageView(VkImageViewType viewType,
                                             VkFormat format,
                                             uint32_t mipLevelCount,
                                             VkImageViewCreateFlags flags) {
  view.Create(memory.Image(), viewType, format,
              {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, 1}, flags);
}

// Static Function
/*CheckArguments(...) should only be called in tests*/
bool vulkanWrapper::texture::CheckArguments(
    VkImageType imageType, VkExtent3D extent, uint32_t arrayLayerCount,
    VkFormat format_initial, VkFormat format_final, bool generateMipmap) {
  auto AliasedImageAvailability =
      [](VkImageType imageType, VkFormat format, VkExtent3D extent,
         uint32_t arrayLayerCount, VkImageUsageFlags usage) {
        if (!(graphic::Plus().FormatProperties(format).linearTilingFeatures &
              VK_FORMAT_FEATURE_BLIT_SRC_BIT))
          return false;
        VkImageFormatProperties imageFormatProperties = {};
        vkGetPhysicalDeviceImageFormatProperties(
            graphic::Singleton().PhysicalDevice(), format, imageType,
            VK_IMAGE_TILING_LINEAR, usage, 0, &imageFormatProperties);
        VkDeviceSize imageDataSize =
            VkDeviceSize(formatInfo::FormatInfo(format).sizePerPixel) *
            extent.width * extent.height * extent.depth;
        return extent.width <= imageFormatProperties.maxExtent.width &&
               extent.height <= imageFormatProperties.maxExtent.height &&
               extent.depth <= imageFormatProperties.maxExtent.depth &&
               arrayLayerCount <= imageFormatProperties.maxArrayLayers &&
               imageDataSize <= imageFormatProperties.maxResourceSize;
      };
  if (graphic::Plus().FormatProperties(format_final).optimalTilingFeatures &
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
    // Case: Copy data from pre-initialized image to final image
    if (graphic::Plus().FormatProperties(format_initial).linearTilingFeatures &
        VK_FORMAT_FEATURE_BLIT_SRC_BIT)
      if (AliasedImageAvailability(imageType, format_initial, extent,
                                   arrayLayerCount,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
        if (graphic::Plus()
                    .FormatProperties(format_final)
                    .optimalTilingFeatures &
                VK_FORMAT_FEATURE_BLIT_DST_BIT &&
            generateMipmap * (graphic::Plus()
                                  .FormatProperties(format_final)
                                  .optimalTilingFeatures &
                              VK_FORMAT_FEATURE_BLIT_SRC_BIT))
          return true;
    // Case: Copy data from staging buffer to final image
    if (format_initial == format_final)
      return graphic::Plus()
                     .FormatProperties(format_final)
                     .optimalTilingFeatures &
                 VK_FORMAT_FEATURE_TRANSFER_DST_BIT &&
             generateMipmap * (graphic::Plus()
                                   .FormatProperties(format_final)
                                   .optimalTilingFeatures &
                               (VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                                VK_FORMAT_FEATURE_BLIT_DST_BIT));
    // Case: Copy data from staging buffer to initial image, then blit initial
    // image to final image
    else
      return graphic::Plus()
                     .FormatProperties(format_initial)
                     .optimalTilingFeatures &
                 (VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                  VK_FORMAT_FEATURE_TRANSFER_DST_BIT) &&
             graphic::Plus()
                     .FormatProperties(format_final)
                     .optimalTilingFeatures &
                 VK_FORMAT_FEATURE_BLIT_DST_BIT &&
             generateMipmap * (graphic::Plus()
                                   .FormatProperties(format_final)
                                   .optimalTilingFeatures &
                               VK_FORMAT_FEATURE_BLIT_SRC_BIT);
  }
  return false;
}
