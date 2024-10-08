#include "imageOperation.hpp"

#include "../Models/graphicPlus.hpp"

#include <cstdint>
#include <stb/stb_image.h>

#include <memory>
#include <vulkan/vulkan_core.h>

void imageOperation::CmdCopyBufferToImage(
    VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
    const VkBufferImageCopy &region, imageMemoryBarrierParameterPack imb_from,
    imageMemoryBarrierParameterPack imb_to) {
  // Pre-copy barrier
  VkImageMemoryBarrier imageMemoryBarrier = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      imb_from.access,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      imb_from.layout,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_QUEUE_FAMILY_IGNORED, // No ownership transfer
      VK_QUEUE_FAMILY_IGNORED,
      image,
      {region.imageSubresource.aspectMask, region.imageSubresource.mipLevel, 1,
       region.imageSubresource.baseArrayLayer,
       region.imageSubresource.layerCount}};
  if (imb_from.isNeeded)
    vkCmdPipelineBarrier(commandBuffer, imb_from.stage,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &imageMemoryBarrier);
  // Copy
  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  // Post-copy barrier
  if (imb_to.isNeeded) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.dstAccessMask = imb_to.access;
    imageMemoryBarrier.newLayout = imb_to.layout;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         imb_to.stage, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
  }
}
void imageOperation::CmdBlitImage(VkCommandBuffer commandBuffer,
                                  VkImage image_src, VkImage image_dst,
                                  const VkImageBlit &region,
                                  imageMemoryBarrierParameterPack imb_dst_from,
                                  imageMemoryBarrierParameterPack imb_dst_to,
                                  VkFilter filter) {
  // Pre-blit barrier
  VkImageMemoryBarrier imageMemoryBarrier = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      imb_dst_from.access,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      imb_dst_from.layout,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      image_dst,
      {region.dstSubresource.aspectMask, region.dstSubresource.mipLevel, 1,
       region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount}};
  if (imb_dst_from.isNeeded)
    vkCmdPipelineBarrier(commandBuffer, imb_dst_from.stage,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &imageMemoryBarrier);
  // Blit
  vkCmdBlitImage(commandBuffer, image_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 image_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region,
                 filter);
  // Post-blit barrier
  if (imb_dst_to.isNeeded) {
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.dstAccessMask = imb_dst_to.access;
    imageMemoryBarrier.newLayout = imb_dst_to.layout;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         imb_dst_to.stage, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
  }
}
void imageOperation::CmdGenerateMipmap2d(VkCommandBuffer commandBuffer,
                                         VkImage image, VkExtent2D imageExtent,
                                         uint32_t mipLevelCount,
                                         uint32_t layerCount,
                                         imageMemoryBarrierParameterPack imb_to,
                                         VkFilter minFilter) {
  auto MipmapExtent = [](VkExtent2D imageExtent, uint32_t mipLevel) {
    VkOffset3D extent = {int32_t(imageExtent.width >> mipLevel),
                         int32_t(imageExtent.height >> mipLevel), 1};
    extent.x += !extent.x;
    extent.y += !extent.y;
    return extent;
  };
  // Blit
  if (layerCount > 1) {
    std::unique_ptr<VkImageBlit[]> regions =
        std::make_unique<VkImageBlit[]>(layerCount);
    for (uint32_t i = 1; i < mipLevelCount; i++) {
      VkOffset3D mipmapExtent_src = MipmapExtent(imageExtent, i - 1);
      VkOffset3D mipmapExtent_dst = MipmapExtent(imageExtent, i);
      for (uint32_t j = 0; j < layerCount; j++)
        regions[j] = {
            {VK_IMAGE_ASPECT_COLOR_BIT, i - 1, j, 1}, // srcSubresource
            {{}, mipmapExtent_src},                   // srcOffsets
            {VK_IMAGE_ASPECT_COLOR_BIT, i, j, 1},     // dstSubresource
            {{}, mipmapExtent_dst}                    // dstOffsets
        };
      // Pre-blit barrier
      VkImageMemoryBarrier imageMemoryBarrier = {
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          nullptr,
          0,
          VK_ACCESS_TRANSFER_WRITE_BIT,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_QUEUE_FAMILY_IGNORED,
          VK_QUEUE_FAMILY_IGNORED,
          image,
          {VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, layerCount}};
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &imageMemoryBarrier);
      // Blit
      vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount,
                     regions.get(), minFilter);
      // Post-blit barrier
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &imageMemoryBarrier);
    }
  } else
    for (uint32_t i = 1; i < mipLevelCount; i++) {
      VkImageBlit region = {
          {VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, layerCount}, // srcSubresource
          {{}, MipmapExtent(imageExtent, i - 1)},            // srcOffsets
          {VK_IMAGE_ASPECT_COLOR_BIT, i, 0, layerCount},     // dstSubresource
          {{}, MipmapExtent(imageExtent, i)}                 // dstOffsets
      };
      CmdBlitImage(
          commandBuffer, image, image, region,
          {VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
          {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
          minFilter);
    }
  // Post-blit barrier
  if (imb_to.isNeeded) {
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        0,
        imb_to.access,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        imb_to.layout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, layerCount}};
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         imb_to.stage, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
  }
}

// Method for handling const uint8_t* address
[[nodiscard]]
std::unique_ptr<uint8_t[]> imageOperation::LoadFile_MemoryAddress(
    const uint8_t *address, size_t fileSize, VkExtent2D &extent,
    formatInfo requiredFormatInfo, int *layerCount) {
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
std::unique_ptr<uint8_t[]>
imageOperation::LoadFile_FileSystem(const char *address, VkExtent2D &extent,
                                    formatInfo requiredFormatInfo,
                                    uint32_t *layerCount) {
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

  if (layerCount != nullptr) {
    if (height % width == 0) {
      *layerCount = height / width;
    } else {
      { *layerCount = 1; }
    }
  }

  if (!pImageData)
    printf("[ texture ] ERROR: Failed to load the file: %s\n", address);

  return std::unique_ptr<uint8_t[]>(static_cast<uint8_t *>(pImageData));
}

void imageOperation::CopyBlitAndGenerateMipmap2d(
    VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo,
    VkFormat sourceFormat, VkExtent2D imageExtent, uint32_t mipLevelCount,
    uint32_t layerCount, VkFilter minFilter) {
  static constexpr imageOperation::imageMemoryBarrierParameterPack imbs[2] = {
      {
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          VK_ACCESS_SHADER_READ_BIT,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },
      {
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_ACCESS_TRANSFER_READ_BIT,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      },
  };
  bool generateMipmap = mipLevelCount > 1;
  bool blitMipLevel0 = image_copyTo != image_blitTo;
  auto &commandBuffer = graphic::Plus().CommandBuffer_Transfer();
  commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  for (uint32_t i = 0; i < layerCount; i++) {
    VkBufferImageCopy region = {
        .bufferOffset = i * imageExtent.width * imageExtent.height *
                        formatInfo::FormatInfo(sourceFormat).sizePerPixel,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = i,
                .layerCount = 1,
            },
        .imageExtent = {imageExtent.width, imageExtent.height, 1}};
    imageOperation::CmdCopyBufferToImage(
        commandBuffer, buffer_copyFrom, image_copyTo, region,
        {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
        imbs[generateMipmap || blitMipLevel0]);
  }
  // Blit to another image if necessary
  if (blitMipLevel0) {
    VkImageBlit region = {
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
        {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
        {{}, {int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
    };
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
void imageOperation::BlitAndGenerateMipmap2d(
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
