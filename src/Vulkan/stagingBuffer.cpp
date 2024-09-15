#include "stagingBuffer.hpp"
#include "formatInfo.hpp"

#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "../Models/graphic.hpp"
#include "../Models/graphicPlus.hpp"

std::unordered_map<std::thread::id, vulkanWrapper::stagingBuffer *>
    vulkanWrapper::stagingBuffer::buffers;
std::shared_mutex vulkanWrapper::stagingBuffer::buffersMutex;

vulkanWrapper::stagingBuffer::stagingBuffer() {
  buffers = {};
  buffers[std::this_thread::get_id()] = this;
}
vulkanWrapper::stagingBuffer::stagingBuffer(VkDeviceSize size) { Expand(size); }
// Const Function
void vulkanWrapper::stagingBuffer::RetrieveData(void *pData_src,
                                                VkDeviceSize size) const {
  memory.RetrieveData(pData_src, size);
}
// Non-const Function
void vulkanWrapper::stagingBuffer::Expand(VkDeviceSize size) {
  if (size <= AllocationSize())
    return;
  Release();
  VkBufferCreateInfo bufferCreateInfo = {.size = size,
                                         .usage =
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT};
  memory.Create(bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}
void vulkanWrapper::stagingBuffer::Release() { memory.~bufferMemory(); }
void *vulkanWrapper::stagingBuffer::MapMemory(VkDeviceSize size) {
  Expand(size);
  void *pData_dst = nullptr;
  memory.MapMemory(pData_dst, size);
  memoryUsage = size;
  return pData_dst;
}
void vulkanWrapper::stagingBuffer::UnmapMemory() {
  memory.UnmapMemory(memoryUsage);
  memoryUsage = 0;
}
void vulkanWrapper::stagingBuffer::BufferData(const void *pData_src,
                                              VkDeviceSize size) {
  Expand(size);
  memory.BufferData(pData_src, size);
}
[[nodiscard]]
VkImage vulkanWrapper::stagingBuffer::AliasedImage2d(VkFormat format,
                                                     VkExtent2D extent) {
  if (!(graphic::Plus().FormatProperties(format).linearTilingFeatures &
        VK_FORMAT_FEATURE_BLIT_SRC_BIT))
    return VK_NULL_HANDLE;
  VkDeviceSize imageDataSize =
      VkDeviceSize(formatInfo::FormatInfo(format).sizePerPixel) * extent.width *
      extent.height;
  if (imageDataSize > AllocationSize())
    return VK_NULL_HANDLE;
  VkImageFormatProperties imageFormatProperties = {};
  vkGetPhysicalDeviceImageFormatProperties(
      graphic::Singleton().PhysicalDevice(), format, VK_IMAGE_TYPE_2D,
      VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0,
      &imageFormatProperties);
  if (extent.width > imageFormatProperties.maxExtent.width ||
      extent.height > imageFormatProperties.maxExtent.height ||
      imageDataSize > imageFormatProperties.maxResourceSize)
    return VK_NULL_HANDLE;
  VkImageCreateInfo imageCreateInfo = {
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {extent.width, extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_LINEAR,
      .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED};
  aliasedImage.~image();
  aliasedImage.Create(imageCreateInfo);
  VkImageSubresource subResource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
  VkSubresourceLayout subresourceLayout = {};
  vkGetImageSubresourceLayout(graphic::Singleton().Device(), aliasedImage,
                              &subResource, &subresourceLayout);
  if (subresourceLayout.size != imageDataSize)
    return VK_NULL_HANDLE;
  aliasedImage.BindMemory(memory.Memory());
  return aliasedImage;
}
