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
                                             uint32_t arrayLayerCount,
                                             VkImageViewCreateFlags flags) {
  view.Create(memory.Image(), viewType, format,
              {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, arrayLayerCount},
              flags);
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

// Method for handling const uint8_t* address
[[nodiscard]]
std::unique_ptr<uint8_t[]> vulkanWrapper::texture::LoadFile_MemoryAddress(
    const uint8_t *address, size_t fileSize, VkExtent2D &extent,
    formatInfo requiredFormatInfo) {
  int &width = reinterpret_cast<int &>(extent.width);
  int &height = reinterpret_cast<int &>(extent.height);
  int channelCount;
  void *pImageData = nullptr;

  if (fileSize > INT32_MAX) {
    printf("[ texture ] ERROR: Failed to load image data from the given "
           "address! Data size must be less than 2G! Given size: %zu\n",
           fileSize);
    return {};
  }

  if (requiredFormatInfo.rawDataType == formatInfo::integer)
    if (requiredFormatInfo.sizePerComponent == 1)
      pImageData = stbi_load_from_memory(address, fileSize, &width, &height,
                                         &channelCount,
                                         requiredFormatInfo.componentCount);
    else
      pImageData = stbi_load_16_from_memory(address, fileSize, &width, &height,
                                            &channelCount,
                                            requiredFormatInfo.componentCount);
  else
    pImageData = stbi_loadf_from_memory(address, fileSize, &width, &height,
                                        &channelCount,
                                        requiredFormatInfo.componentCount);

  if (!pImageData)
    printf("[ texture ] ERROR: Failed to load image data "
           "from the given address! Given address: %p\n",
           address);

  return std::unique_ptr<uint8_t[]>(static_cast<uint8_t *>(pImageData));
}

// Method for handling const char* address
[[nodiscard]]
std::unique_ptr<uint8_t[]> vulkanWrapper::texture::LoadFile_FileSystem(
    const char *address, VkExtent2D &extent, formatInfo requiredFormatInfo) {
  int &width = reinterpret_cast<int &>(extent.width);
  int &height = reinterpret_cast<int &>(extent.height);
  int channelCount;
  void *pImageData = nullptr;

  if (requiredFormatInfo.rawDataType == formatInfo::integer)
    if (requiredFormatInfo.sizePerComponent == 1)
      pImageData = stbi_load(address, &width, &height, &channelCount,
                             requiredFormatInfo.componentCount);
    else
      pImageData = stbi_load_16(address, &width, &height, &channelCount,
                                requiredFormatInfo.componentCount);
  else
    pImageData = stbi_loadf(address, &width, &height, &channelCount,
                            requiredFormatInfo.componentCount);

  if (!pImageData)
    printf("[ texture ] ERROR: Failed to load the file: %s\n", address);

  return std::unique_ptr<uint8_t[]>(static_cast<uint8_t *>(pImageData));
}

void vulkanWrapper::texture::CopyBlitAndGenerateMipmap2d(
    VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo,
    VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount,
    VkFilter minFilter) {
  static constexpr imageOperation::imageMemoryBarrierParameterPack imbs[2] = {
      {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL}};
  bool generateMipmap = mipLevelCount > 1;
  bool blitMipLevel0 = image_copyTo != image_blitTo;
  auto &commandBuffer = graphic::Plus().CommandBuffer_Transfer();
  commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VkBufferImageCopy region = {
      .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
      .imageExtent = {imageExtent.width, imageExtent.height, 1}};
  imageOperation::CmdCopyBufferToImage(
      commandBuffer, buffer_copyFrom, image_copyTo, region,
      {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
      imbs[generateMipmap || blitMipLevel0]);
  // Blit to another image if necessary
  if (blitMipLevel0) {
    VkImageBlit region = {
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
        {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
        {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}}};
    imageOperation::CmdBlitImage(
        commandBuffer, image_copyTo, image_blitTo, region,
        {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
        imbs[generateMipmap], minFilter);
  }
  // Generate mipmap if necessary, transition layout
  if (generateMipmap)
    imageOperation::CmdGenerateMipmap2d(
        commandBuffer, image_blitTo, imageExtent, mipLevelCount, layerCount,
        {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        minFilter);
  commandBuffer.End();
  // Submit
  graphic::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
}
void vulkanWrapper::texture::BlitAndGenerateMipmap2d(
    VkImage image_preinitialized, VkImage image_final, VkExtent2D imageExtent,
    uint32_t mipLevelCount, uint32_t layerCount, VkFilter minFilter) {
  static constexpr imageOperation::imageMemoryBarrierParameterPack imbs[2] = {
      {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL}};
  bool generateMipmap = mipLevelCount > 1;
  bool blitMipLevel0 = image_preinitialized != image_final;
  if (generateMipmap || blitMipLevel0) {
    auto &commandBuffer = graphic::Plus().CommandBuffer_Transfer();
    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    // Blit to another image if necessary
    if (blitMipLevel0) {
      VkImageMemoryBarrier imageMemoryBarrier = {
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          nullptr,
          0,
          VK_ACCESS_TRANSFER_READ_BIT,
          VK_IMAGE_LAYOUT_PREINITIALIZED,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          VK_QUEUE_FAMILY_IGNORED,
          VK_QUEUE_FAMILY_IGNORED,
          image_preinitialized,
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount}};
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &imageMemoryBarrier);
      VkImageBlit region = {
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
          {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
          {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}}};
      imageOperation::CmdBlitImage(
          commandBuffer, image_preinitialized, image_final, region,
          {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
          imbs[generateMipmap], minFilter);
    }
    // Generate mipmap if necessary, transition layout
    if (generateMipmap)
      imageOperation::CmdGenerateMipmap2d(
          commandBuffer, image_final, imageExtent, mipLevelCount, layerCount,
          {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
          minFilter);
    commandBuffer.End();
    // Submit
    graphic::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
  }
}
