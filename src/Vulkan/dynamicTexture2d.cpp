#include "dynamicTexture2d.hpp"

#include "../Utils/imageOperation.hpp"

#include "../Vulkan/formatInfo.hpp"
#include "stagingBuffer.hpp"
#include <vulkan/vulkan_core.h>

// 直接从硬盘读取文件
void vulkanWrapper::dynamicTexture2d::Create(const char *filepath,
                                             VkFormat format_initial,
                                             VkFormat format_final,
                                             bool generateMipmap,
                                             VkFilter blitFilter) {
  VkExtent2D extent;
  formatInfo formatInfo = formatInfo::FormatInfo(format_initial);
  uint32_t layerCount = 1;
  std::unique_ptr<uint8_t[]> data = imageOperation::LoadFile_FileSystem(
      filepath, extent, formatInfo, &layerCount);
  extent.height /= layerCount;
  if (data) {
    Create(data.get(), extent, layerCount, format_initial, format_final,
           generateMipmap, blitFilter);
  }
}
// 从内存读取文件数据
void vulkanWrapper::dynamicTexture2d::Create(
    const uint8_t *pImageData, VkExtent2D extent, uint32_t layerCount,
    VkFormat format_initial, VkFormat format_final, bool generateMipmap,
    VkFilter blitFilter) {
  this->extent = extent;
  size_t imageSize =
      size_t(formatInfo::FormatInfo(format_initial).sizePerPixel) *
      extent.width * extent.height;
  stagingBuffer::BufferData_CurrentThread(pImageData, imageSize * layerCount);
  Create_Internal(format_initial, format_final, generateMipmap, layerCount,
                  blitFilter);
}

void vulkanWrapper::dynamicTexture2d::Create_Internal(VkFormat format_initial,
                                                      VkFormat format_final,
                                                      bool generateMipmap,
                                                      uint32_t layerCount,
                                                      VkFilter blitFilter) {
  uint32_t mipLevelCount =
      generateMipmap ? imageOperation::CalculateMipLevelCount(extent) : 1;
  // 创建图像并分配内存
  CreateImageMemory(VK_IMAGE_TYPE_2D, format_final,
                    {extent.width, extent.height, 1}, mipLevelCount,
                    layerCount);
  // 创建图像视图
  CreateImageViews(VK_IMAGE_VIEW_TYPE_2D, format_final, mipLevelCount,
                   layerCount);
  // Blit数据到图像，并生成mipmap
  if (format_initial == format_final)
    imageOperation::CopyBlitAndGenerateMipmap2d(
        stagingBuffer::Buffer_CurrentThread(), memory.Image(), memory.Image(),
        format_initial, extent, mipLevelCount, layerCount, blitFilter);
  else if (VkImage image_conversion =
               stagingBuffer::AliasedImage2d_CurrentThread(format_initial,
                                                           extent))
    imageOperation::BlitAndGenerateMipmap2d(image_conversion, memory.Image(),
                                            extent, mipLevelCount, layerCount,
                                            blitFilter);
  else {
    VkImageCreateInfo imageCreateInfo = {
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_initial,
        .extent = {extent.width, extent.height, 1},
        .mipLevels = 1,
        .arrayLayers = layerCount,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage =
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    vulkanWrapper::imageMemory imageMemory_conversion(
        imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    imageOperation::CopyBlitAndGenerateMipmap2d(
        stagingBuffer::Buffer_CurrentThread(), imageMemory_conversion.Image(),
        memory.Image(), format_initial, extent, mipLevelCount, layerCount,
        blitFilter);
  }
}
