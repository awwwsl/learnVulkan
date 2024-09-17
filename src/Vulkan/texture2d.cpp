#include "texture2d.hpp"
#include "stagingBuffer.hpp"

void vulkanWrapper::texture2d::Create_Internal(VkFormat format_initial,
                                               VkFormat format_final,
                                               bool generateMipmap) {
  uint32_t mipLevelCount = generateMipmap ? CalculateMipLevelCount(extent) : 1;
  // 创建图像并分配内存
  CreateImageMemory(VK_IMAGE_TYPE_2D, format_final,
                    {extent.width, extent.height, 1}, mipLevelCount, 1);
  // 创建图像视图
  CreateImageView(VK_IMAGE_VIEW_TYPE_2D, format_final, mipLevelCount, 1);
  // Blit数据到图像，并生成mipmap
  if (format_initial == format_final)
    CopyBlitAndGenerateMipmap2d(stagingBuffer::Buffer_CurrentThread(),
                                memory.Image(), memory.Image(), extent,
                                mipLevelCount, 1);
  else if (VkImage image_conversion =
               stagingBuffer::AliasedImage2d_CurrentThread(format_initial,
                                                           extent))
    BlitAndGenerateMipmap2d(image_conversion, memory.Image(), extent,
                            mipLevelCount, 1);
  else {
    VkImageCreateInfo imageCreateInfo = {
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_initial,
        .extent = {extent.width, extent.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage =
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    vulkanWrapper::imageMemory imageMemory_conversion(
        imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CopyBlitAndGenerateMipmap2d(stagingBuffer::Buffer_CurrentThread(),
                                imageMemory_conversion.Image(), memory.Image(),
                                extent, mipLevelCount, 1);
  }
}

vulkanWrapper::texture2d ::texture2d() = default;
// Non-const Function
// 直接从硬盘读取文件
void vulkanWrapper::texture2d::Create(const char *filepath,
                                      VkFormat format_initial,
                                      VkFormat format_final,
                                      bool generateMipmap) {
  VkExtent2D extent;
  formatInfo formatInfo = formatInfo::FormatInfo(
      format_initial); // 根据指定的format_initial取得格式信息
  std::unique_ptr<uint8_t[]> pImageData =
      LoadFile_FileSystem(filepath, extent, formatInfo);
  if (pImageData)
    Create(pImageData.get(), extent, format_initial, format_final,
           generateMipmap);
}
// 从内存读取文件数据
void vulkanWrapper::texture2d::Create(const uint8_t *pImageData,
                                      VkExtent2D extent,
                                      VkFormat format_initial,
                                      VkFormat format_final,
                                      bool generateMipmap) {
  this->extent = extent;
  size_t imageDataSize =
      size_t(formatInfo::FormatInfo(format_initial).sizePerPixel) *
      extent.width * extent.height;
  stagingBuffer::BufferData_CurrentThread(
      pImageData,
      imageDataSize); // 拷贝数据到暂存缓冲区
  Create_Internal(format_initial, format_final, generateMipmap);
}
